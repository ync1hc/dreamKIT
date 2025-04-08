#define USING_VSS

#ifdef USING_VSS
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")
// Python global object
PyObject *pFunc;
PyObject *pName;
PyObject *pModule;
bool isPyEnvOk = false;
#endif

#include <iostream>
#include <string>

void initPyEnv()
{
#ifdef USING_VSS
    // Initialize the Python Interpreter
    Py_Initialize();

    // Set the Python script's directory (optional)
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")"); // Add the current directory to the Python path

    // Load the script
    pName = PyUnicode_DecodeFSDefault("vssclient");
    if(!pName) {
        isPyEnvOk = false;
        return;
    }

    pModule = PyImport_Import(pName);
    if(!pModule) {
        isPyEnvOk = false;
        return;
    }

    pFunc = PyObject_GetAttrString(pModule, "getCurrentValue");
    if(!pFunc) {
        isPyEnvOk = false;
        return;
    }
    
    isPyEnvOk= true;
#endif
}

void endPyEnv()
{
#ifdef USING_VSS
    Py_Finalize();
#endif
}

u_int32_t getUint32_VssApiValue(std::string apiName, std::string &currentTimeStamp)
{
#ifdef USING_VSS
    if (pModule != nullptr) {

        // Check if the function is callable
        if (PyCallable_Check(pFunc)) {

            // Prepare the arguments for the function call
            PyObject *pArgs = PyTuple_Pack(1, PyUnicode_FromString(apiName.c_str()));

            // Call the function
            std::cout << "before: " << std::endl;
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs); 
            std::cout << "after _ 1: " << std::endl;
            Py_DECREF(pArgs);
            // Check if the result is a tuple
            if (!pValue) return 0;
            if (PyTuple_Check(pValue)) {
                std::cout << "PyTuple_Check True" << std::endl;
                PyObject *pVal = PyTuple_GetItem(pValue, 0);
                PyObject *pTime = PyTuple_GetItem(pValue, 1);                

                if (pVal && pTime) {
                    u_int32_t result = PyLong_AsUnsignedLong(pVal);
                    const char *timestampCStr = PyUnicode_AsUTF8(pTime);
                    std::cout << "value return: " << result << std::endl;

                    if (timestampCStr) {
                        std::cout << "has timestamp " << std::endl;
                        std::string timestamp = timestampCStr;
                        currentTimeStamp = timestamp;
                        std::cout << "timestamp: " <<  timestamp << std::endl;
                        Py_DECREF(pValue);
                        // return std::make_tuple(QString::fromStdString(result), QString::fromStdString(timestamp));
                        return result;
                    }
                 }
            }
            else {
                std::cout << "PyTuple_Check False" << std::endl;
                if (pValue != nullptr) {

                    if (currentTimeStamp == "")  {
                        return 0;
                    }
                        
                    // Convert the result to a C++ bool
                    u_int32_t result = PyLong_AsUnsignedLong(pValue);
                    std::cout << "value return: " << result << std::endl;
                    Py_DECREF(pValue);
                    return result;
                } else {
                    PyErr_Print();
                    std::cerr << "Call to get_tts_text failed" << std::endl;
                }
            }            
        } else {
            if (PyErr_Occurred()) PyErr_Print();
            std::cerr << "Cannot find function 'get_tts_text'" << std::endl;
        }        
    } else {
        PyErr_Print();
        std::cerr << "Failed to load 'vssclient'" << std::endl;
    } 
#endif
    return 0;
}

int main() {
    initPyEnv();
    std::string vapi = "Vehicle.Security.IDPS.VehicleReaction";
    std::string timestamp = "";
    u_int32_t value = getUint32_VssApiValue(vapi, timestamp);
    std::cout << "value: " << value << std::endl;
    endPyEnv();
    return 0;
}
