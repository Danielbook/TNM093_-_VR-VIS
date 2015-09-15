#ifndef VRN_TNM_CSVLOADER_H
#define VRN_TNM_CSVLOADER_H

#include "voreen/core/processors/processor.h"
#include "modules/tnm093/include/tnm_common.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

class TNMCSVLoader : public Processor {
public:
    TNMCSVLoader();
    ~TNMCSVLoader();
    std::string getClassName() const   { return "TNMCSVLoader";   }
    std::string getCategory() const    { return "tnm093"               ;  }
    CodeState getCodeState() const     { return CODE_STATE_EXPERIMENTAL;  }

    Processor* create() const          { return new TNMCSVLoader; }

protected:
    void process();

private:
	void loadFile();

    DataPort _outport; // The outport containing the computed measures

	FileDialogProperty _csvFile;
	ButtonProperty _loadButton;

    Data* _data; // The local copy of the computed data; ownership stays with this object at all times
};

} // namespace

#endif // VRN_TNM_VOLUMEINFORMATION_H
