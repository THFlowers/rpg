#include <python2.7/Python.h>
#include "python2.7/structmember.h"

#ifndef RPG_PYTHON
#define RPG_PYTHON

#ifndef PyModINIT_FUNC  /* declarations for DLL import/export */
#define PyModINIT_FUNC void
#endif

PyModINIT_FUNC initrpg(void);
static PyObject* PY_Test(PyObject *self, PyObject *args);

#endif
