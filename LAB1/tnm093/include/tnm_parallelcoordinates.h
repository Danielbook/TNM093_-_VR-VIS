#ifndef VRN_TNM_PARALLELCOORDINATES_H
#define VRN_TNM_PARALLELCOORDINATES_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "modules/tnm093/include/tnm_common.h"
#include "modules/tnm093/include/indexproperty.h"
#include "voreen/core/properties/fontproperty.h"
#include "tgt/vector.h"
#include <utility>
#include <vector>

namespace voreen {

class TNMParallelCoordinates : public RenderProcessor {
public:
	// This class stores and renders a single handle
	// it provides access to the index for picking
	class AxisHandle {
	public:
		// The position, where the handle is going to be. Determines the orientation of the
		// trianglular shape
		enum AxisHandlePosition {
			AxisHandlePositionTop = 0,
			AxisHandlePositionBottom
		};

		// location: if the axis handle is on the top or bottom part of the axis
		// index: a unique index that gets encoded in the red channel of the picking texture
		// position: the position (in [-1,1]) where the axis handle will be drawn
		AxisHandle(AxisHandlePosition location, int index, const tgt::vec2& position);

		// Returns the index of the axis handle
		int index() const;

		// Sets the new position of the axis handle
		void setPosition(const tgt::vec2& position);

		tgt::vec2 position() const;

		// Renders the handle at the current position with the color meant for presentation
		void render() const;

		// Renders the handle at the current position using the encoded picking color
		void renderPicking() const;
		
		// These are used internally in order to reuse the VBO and Shaders
		void setVertexBufferObjectAndShader(GLuint vbo, tgt::Shader* shader);

		AxisHandlePosition location() const;
		

	private:
		// The internal method that gets called by both render() and renderPicking() methods
		void renderInternal(const tgt::vec3& color) const;

		// The location of the handle
		AxisHandlePosition _location;

		// The unique index of the handle
		int _index;

		// The current position of the handle
		tgt::vec2 _position;

		GLuint _vbo;
		tgt::Shader* _shader;
	};

    TNMParallelCoordinates();
    ~TNMParallelCoordinates();
    std::string getClassName() const   { return "TNMParallelCoordinates";   }
    std::string getCategory() const    { return "tnm093"               ; }
    CodeState getCodeState() const     { return CODE_STATE_EXPERIMENTAL; }

    Processor* create() const          { return new TNMParallelCoordinates; }

	void initialize() throw (tgt::Exception);
	void deinitialize() throw (tgt::Exception);

protected:
	// This method gets called during each run of the rendering loop
    void process();

	// Render the text for the handles
	void renderText();

	// Render the lines for the parallel coordinates plot
    void renderLines();

	// Render the lines with picking information included in the color
	void renderLinesPicking();

	// Render the handles of the parallel coordinate axes
    void renderHandles();

	// Render the handles of the parallel coordinates axes with picking information
	// included in the color
    void renderHandlesPicking();
        
	// The callback method that gets called when a mouse button was clicked on the rendering
    void handleMouseClick(tgt::MouseEvent* e);

    
    void handleMouseClickRight(tgt::MouseEvent* e);
    
	// The callback method that gets called when the mouse is moved across the rendering
    void handleMouseMove(tgt::MouseEvent* e);

	// The callback method that gets called when the mouse button is released
    void handleMouseRelease(tgt::MouseEvent* e);
    
private:
	// The inport supplying the data to this processor
    DataPort _inport;

	// The outport that will contain the rendering meant for the user
    RenderPort _outport;

	// The picking port which will be rendered to with the encoded IDs
	// and which will be queried in the mouse callbacks
    RenderPort _privatePort;

	// The event that registers the click event
    EventProperty<TNMParallelCoordinates>* _mouseClickEvent;
    
    EventProperty<TNMParallelCoordinates>* _mouseClickEventRight;
	// The event that registers the move event
    EventProperty<TNMParallelCoordinates>* _mouseMoveEvent;
	// The event that registeres the release event
    EventProperty<TNMParallelCoordinates>* _mouseReleaseEvent;

	GLuint _handleVBO;
	tgt::Shader* _handleShader;

	FontProperty _font;

	// A list of all the handles that are registerd
    std::vector<AxisHandle> _handles;
	// The id of the currently picked handle; necessary in the interaction between the
	// mouseClick and the mouseMove methods to store the ID that was clicked
    int _pickedHandle;
	
	IntOptionProperty _colorMethod;

	//
	IndexProperty _brushingIndices;  // A list of voxel indices that should be ignored in the rendering
	IndexProperty _linkingIndices; // A list of voxel indices that should be enhanced during rendering

	std::set<unsigned int> _brushingList; // The internal storage for the list of ignored voxels
	std::set<unsigned int> _linkingList; // The internal storage for the list of selected voxels
};

} // namespace

#endif // VRN_TNM_PARALLELCOORDINATES_H
