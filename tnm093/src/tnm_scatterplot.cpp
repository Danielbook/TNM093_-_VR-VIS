#include "modules/tnm093/include/tnm_scatterplot.h"

#include <algorithm>
#include <limits>

namespace voreen {

TNMScatterPlot::TNMScatterPlot()
    : RenderProcessor()
    , _inport(Port::INPORT, "in.data")
    , _outport(Port::OUTPORT, "out.image")
	, _shader(0)
    , _firstAxis("firstAxis", "Horizontal Axis")
    , _secondAxis("secondAxis", "Vertical Axis")
	, _pointSize("pointSize", "Point Size", 1, 1, 5)
	, _brushingIndices("brushingIndices", "Brushing Indices")
	, _linkingIndices("linkingIndices", "Linking Indices")
{
    addPort(_inport);
    addPort(_outport);

    addProperty(_firstAxis);
    addProperty(_secondAxis);
	addProperty(_pointSize);
	addProperty(_brushingIndices);
	addProperty(_linkingIndices);

}

void TNMScatterPlot::initialize() throw (tgt::Exception) {
	// Load the shaders and return the pointer to the shader program
	_shader = ShdrMgr.loadSeparate("scatterplot.vert", "scatterplot.frag");

	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_selectionVbo);
}

void TNMScatterPlot::deinitialize() throw (tgt::Exception) {
	ShdrMgr.dispose(_shader);

	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_selectionVbo);
}

void TNMScatterPlot::process() {
    if (!_inport.hasData())
        return;
	if (_inport.getData()->minimumMaximumValues.empty())
		return;

	// Access the provided data. We have already checked before that it exists, so dereferencing it here is safe
	const Data& data = *(_inport.getData());

	if (_inport.hasChanged()) {
        _firstAxis.setOptions(std::vector<Option<int> >());
        _secondAxis.setOptions(std::vector<Option<int> >());
		for (size_t i = 0; i < data.valueNames.size(); ++i) {
			std::stringstream s;
			s << i;
			_firstAxis.addOption(s.str(), data.valueNames[i], i);
			_secondAxis.addOption(s.str(), data.valueNames[i], i);
			s.str("");
		}
		_firstAxis.updateWidgets();
		_secondAxis.updateWidgets();
	}

	// Activate the outport as the rendering target
    _outport.activateTarget();
	// Clear the buffer
    _outport.clearTarget();

	// The set contains all indices of voxels that should be ignored
	const std::set<unsigned int>& brushingIndices = _brushingIndices.get();
	// The set contains all indices of voxels that should be visually selected
	const std::set<unsigned int>& selectionIndices = _linkingIndices.get();

	// The number of points is equal to the number in the original dataset minus the number we are ignoring
	const size_t dataSize = data.data.size() - brushingIndices.size();
	// There are 2 coordinate components for each point
	const size_t nCoordinateComponents = dataSize * 2;

	// The vector containing the position data
	std::vector<float> positionData;
	// Make room in the vector
	positionData.resize(nCoordinateComponents); 

	// The vector containing boolean flags saying for each position if it is selected
	// OpenGL doesn't support boolean values for the vertex buffer, so we take the next best thing instead
	std::vector<unsigned char> selectionData(dataSize, 0);
	
	// In order to map the value ranges to [-1,1] we need to find the mininum and maximum values
	float minimumFirstCoordinate = data.minimumMaximumValues[_firstAxis.getValue()].first;
	float maximumFirstCoordinate = data.minimumMaximumValues[_firstAxis.getValue()].second;
	float minimumSecondCoordinate = data.minimumMaximumValues[_secondAxis.getValue()].first;
	float maximumSecondCoordinate = data.minimumMaximumValues[_secondAxis.getValue()].second;
	// i: index into the data
	// j: index into the coordinates
	for (size_t i = 0, j = 0; i < data.data.size(); ++i) {
		//// See if the index i is in the vector for brushing
		if (brushingIndices.find(data.data[i].voxelIndex) != brushingIndices.end())
			// If it is, we ignore it
			continue;
		else {
			// otherwise add it to the position data
			// _firstAxis.getValue() and _secondAxis.getValue() returns the integer value specified above
			// to determine which selection was chosen in the GUI
			const float firstCoordinate = data.data[i].dataValues[_firstAxis.getValue()];
			const float secondCoordinate = data.data[i].dataValues[_secondAxis.getValue()];
			positionData[j] = firstCoordinate;
			positionData[j+1] = secondCoordinate;
			j += 2;
		}
	}

	// In a second step, we need to normalize the found data. By now we have looked at each value and found the
	// min/max values
	// Normalizing the data values to the range [-1,1]
	for (size_t i = 0; i < nCoordinateComponents; i+=2) {
		// First normalize to [0,1]
		positionData[i] = (positionData[i] - minimumFirstCoordinate) / (maximumFirstCoordinate - minimumFirstCoordinate);
		positionData[i+1] = (positionData[i+1] - minimumSecondCoordinate) / (maximumSecondCoordinate - minimumSecondCoordinate);
		
		// Then shift the normalized values to [-1,1]
		positionData[i] = (positionData[i] - 0.5f) * 2.f;
		positionData[i+1] = (positionData[i+1] - 0.5f) * 2.f;
	}

	// Set the selection array for all selected indices
	for (std::set<unsigned int>::const_iterator i = selectionIndices.begin();
		i != selectionIndices.end();
		++i) 
	{
		const unsigned int thisIndex = *i;
		selectionData[thisIndex] = 1;
	}

	if (!positionData.empty()) {
		// We want to be able to set the point size from the vertex shader
		glEnable(GL_PROGRAM_POINT_SIZE);

		// Activate and fill the vbo containing the position data
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(float), &(positionData[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Activate, create, and fill the vbo containing the selection data
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, _selectionVbo);
		glBufferData(GL_ARRAY_BUFFER, selectionData.size() * sizeof(unsigned char), &(selectionData[0]), GL_STATIC_DRAW);
		glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, 0);

		// Activate the shader required for rendering
		_shader->activate();

		// Set the minimum size of the displayed points
		_shader->setUniform("_pointSize", _pointSize.get());

		// Draw the points
		glDrawArrays(GL_POINTS, 0, dataSize);
	}

	// And be a good citizen and clean up
	_shader->deactivate();
	glDisable(GL_PROGRAM_POINT_SIZE);
    _outport.deactivateTarget();
}


} // namespace
