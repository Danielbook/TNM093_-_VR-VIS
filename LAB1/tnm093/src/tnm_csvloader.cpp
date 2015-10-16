#include "modules/tnm093/include/tnm_csvloader.h"

#include <fstream>

namespace voreen {
const std::string loggerCat_ = "TNMCSVLoader";
	
namespace {

template<class T>
std::vector<T> tokenize(const std::string& line) {
	std::vector<T> result;
	size_t position = line.find(";");

	if (position == std::string::npos && line.empty()) {
		LERRORC("tokenize", "No ; found and string is empty");
		return std::vector<T>();
	}

	if (position == std::string::npos) {
		// Only one value present
		std::stringstream sstream;
		sstream << line;
		T value;
		sstream >> value;
		result.push_back(value);
	}
	else {
		size_t currentPosition = 0;
		while (position != std::string::npos) {
			std::string substr;
			if (position == std::string::npos)
				substr = line.substr(currentPosition);
			else
				substr = line.substr(currentPosition, position - currentPosition);

			std::stringstream sstream;
			sstream << substr;
			T value;
			sstream >> value;
			result.push_back(value);
			currentPosition = position + 1;
			position = line.find(";", position + 1);
		}

		position = line.rfind(";");
		std::string substr = line.substr(position + 1);
		if (!substr.empty()) {
			std::stringstream sstream;
			sstream << substr;
			T value;
			sstream >> value;
			result.push_back(value);
		}
	}

	return result;
}
}

TNMCSVLoader::TNMCSVLoader()
    : Processor()
    , _outport(Port::OUTPORT, "out.data")
	, _csvFile("file", "CSV File", "CSV File", ".")
	, _loadButton("load", "Load")
    , _data(0)
{
    addPort(_outport);

	addProperty(_csvFile);
	addProperty(_loadButton);

	_loadButton.onChange(CallMemberAction<TNMCSVLoader>(this, &TNMCSVLoader::loadFile));
}

TNMCSVLoader::~TNMCSVLoader() {
    delete _data;
}

void TNMCSVLoader::loadFile() {
	delete _data;
	_data = new Data;

	std::ifstream file(_csvFile.get().c_str());

	std::string line;
	// The first line of the file contains information about the values
	std::getline(file, line);

	_data->valueNames = tokenize<std::string>(line);

	int currentIndex = 0;
	while (std::getline(file, line)) {
		VoxelDataItem item;
		item.voxelIndex = currentIndex++;

		std::vector<float> values = tokenize<float>(line);
		for (size_t i = 0; i < values.size(); ++i)
			item.dataValues[i] = values[i];

		_data->data.push_back(item);
	}

	_data->minimumMaximumValues.resize(_data->valueNames.size());
	for (size_t j = 0; j < _data->minimumMaximumValues.size(); ++j) {
		_data->minimumMaximumValues[j].first = std::numeric_limits<float>::max();
		_data->minimumMaximumValues[j].second = -std::numeric_limits<float>::max();
	}
	for (size_t i = 0; i < _data->data.size(); ++i) {
		for (int j = 0; j < 6; ++j) {
			_data->minimumMaximumValues[j].first = std::min(
				_data->minimumMaximumValues[j].first, _data->data[i].dataValues[j]
			);
			_data->minimumMaximumValues[j].second = std::max(
				_data->minimumMaximumValues[j].second, _data->data[i].dataValues[j]
			);
		}
	}
	_outport.setData(_data, false);

}

void TNMCSVLoader::process() {}


} // namespace
