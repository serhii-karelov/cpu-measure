#define PY_SSIZE_T_CLEAN
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <Python.h>

#define START(start_ptr)           \
  do {                             \
    unsigned int dummy;            \
    *start_ptr = __rdtscp(&dummy); \
  } while(0);

#define END(end_ptr)               \
  do {                             \
    unsigned int dummy;            \
    *end_ptr = __rdtscp(&dummy);   \
  } while(0);

uint64_t measure_overhead() {
  unsigned int dummy;
  uint64_t start = __rdtscp(&dummy);
  uint64_t end = __rdtscp(&dummy);
  return end - start;
}

typedef struct {
  PyObject_HEAD
  uint64_t start;
  uint64_t end;
  uint64_t elapsed_cycles;
  uint64_t overhead;
} MeasureObject;


static int measure_init(PyObject *self, PyObject* args, PyObject kwargs) {
  MeasureObject *m = (MeasureObject *) self;
  m->start = 0;
  m->end = 0;
  m->elapsed_cycles = 0;
  m->overhead = measure_overhead();
  return 0;
}

static void measure_dealloc(PyObject *self) {
  Py_TYPE(self)->tp_free(self); 
}

/* Invariant: start is 0 if we outside of the context manager (i.e. not entered) */
PyObject *measure_enter(PyObject *self, PyObject *args) {
  MeasureObject *m = (MeasureObject *) self;
  if (m->start) {
    PyErr_SetString(PyExc_RuntimeError, "Already entered the context");
    return NULL;
  }
  START(&(m->start));
  Py_INCREF(self);
  return self;
}

PyObject *measure_exit(PyObject *self, PyObject *args) {
  uint64_t end;
  END(&end);
  MeasureObject *m = (MeasureObject *) self;
  if (m->start == 0) {
    PyErr_SetString(PyExc_RuntimeError, "Not within a context");
    return NULL;
  }
  m->elapsed_cycles += end - m->start;
  m->start = 0;
  return Py_None;
}
static PyMemberDef measure_members[] = {
  {"cycles", Py_T_ULONG, offsetof(MeasureObject, elapsed_cycles), Py_READONLY, "Number of cycles elapsed between start and end"},
  {NULL}, /* TERMINATOR */
};

static PyMethodDef measure_methods[] = {
  {"__enter__", measure_enter, METH_VARARGS, NULL},
  {"__exit__", measure_exit, METH_VARARGS, NULL},
  {NULL, NULL, 0, NULL}, /* TERMINATOR */
};

static PyTypeObject MeasureType = {
  .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "measure.Measure",
  .tp_basicsize = sizeof(MeasureObject),
  .tp_itemsize = 0,
  .tp_new = PyType_GenericNew,
  .tp_init = (initproc) measure_init,
  .tp_dealloc = (destructor) measure_dealloc,
  .tp_members = measure_members,
  .tp_methods = measure_methods,
};

static struct PyModuleDef measuremodule = {
  PyModuleDef_HEAD_INIT,
  "Measure elapsed clock cycles", 
  NULL, 
  -1,
};

PyMODINIT_FUNC PyInit_measure(void) {
  if (PyType_Ready(&MeasureType) < 0) {
    return NULL;
  }
  PyObject *module = PyModule_Create(&measuremodule);  
  if (module == NULL) {
    return NULL;
  }
  Py_INCREF(&MeasureType);
  if (PyModule_AddObject(module, "Measure", (PyObject *) &MeasureType) < 0) {
    Py_DECREF(&MeasureType);
    Py_DECREF(module);
    return NULL;
  }
  return module;
}

