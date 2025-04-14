#include <Python.h>
#include <iostream>
#include <string>

std::string callPythonGetTTS(const std::string& vapi) {
    // Initialize the Python Interpreter
    Py_Initialize();

    // Set the Python script's directory (optional)
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")"); // Add the current directory to the Python path

    // Load the script
    PyObject *pName = PyUnicode_DecodeFSDefault("vssclient");
    PyObject *pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != nullptr) {
        // Get the get_tts_text function from the script
        PyObject *pFunc = PyObject_GetAttrString(pModule, "getVssTargetValue");

        // Check if the function is callable
        if (PyCallable_Check(pFunc)) {
            // Prepare the arguments for the function call
            PyObject *pArgs = PyTuple_Pack(1, PyUnicode_FromString(vapi.c_str()));

            // Call the function
            PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);

            if (pValue != nullptr) {
                // Convert the result to a C++ string
                const char* resultCStr = PyUnicode_AsUTF8(pValue);
                if (resultCStr != nullptr) {
                    std::string result = resultCStr;
                    Py_DECREF(pValue);

                    // Finalize the Python Interpreter
                    Py_Finalize();

                    return result;
                } else {
                    Py_DECREF(pValue);
                    PyErr_Print();
                    std::cerr << "Error: PyUnicode_AsUTF8 returned null" << std::endl;
                }
            } else {
                PyErr_Print();
                std::cerr << "Call to get_tts_text failed" << std::endl;
            }

            Py_DECREF(pFunc);
        } else {
            if (PyErr_Occurred()) PyErr_Print();
            std::cerr << "Cannot find function 'get_tts_text'" << std::endl;
        }

        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        std::cerr << "Failed to load 'vssclient'" << std::endl;
    }

    // Finalize the Python Interpreter
    Py_Finalize();

    return "";
}

int main() {
    std::string vapi = "Vehicle.TextToSpeech";
    std::string ttsText = callPythonGetTTS(vapi);
    std::cout << "Text from Python: " << ttsText << std::endl;

    return 0;
}
