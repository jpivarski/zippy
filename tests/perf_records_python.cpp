#include <stdio.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void run() {
  printf("BEGIN\n");

  PyRun_SimpleString(
    "import sys\n"
    // "from typing import Any\n"
    // "from dataclasses import dataclass\n"
    "\n"
    "print(sys.version)\n"
    "\n"
    // "@dataclass(frozen=True, slots=True)\n"
    // "class class_2fields:\n"
    // "    field1: Any\n"
    // "    field2: Any\n"
    // "@dataclass(frozen=True, slots=True)\n"
    // "class class_3fields:\n"
    // "    field1: Any\n"
    // "    field2: Any\n"
    // "    field3: Any\n"
    // "@dataclass(frozen=True, slots=True)\n"
    // "class class_4fields:\n"
    // "    field1: Any\n"
    // "    field2: Any\n"
    // "    field3: Any\n"
    // "    field4: Any\n"
    // "@dataclass(frozen=True, slots=True)\n"
    // "class class_5fields:\n"
    // "    field1: Any\n"
    // "    field2: Any\n"
    // "    field3: Any\n"
    // "    field4: Any\n"
    // "    field5: Any\n"
    // "@dataclass(frozen=True, slots=True)\n"
    // "class class_6fields:\n"
    // "    field1: Any\n"
    // "    field2: Any\n"
    // "    field3: Any\n"
    // "    field4: Any\n"
    // "    field5: Any\n"
    // "    field6: Any\n"
    "class class_2fields:\n"
    "    def __init__(self, field1, field2):\n"
    "        self.field1 = field1\n"
    "        self.field2 = field2\n"
    "class class_3fields:\n"
    "    def __init__(self, field1, field2, field3):\n"
    "        self.field1 = field1\n"
    "        self.field2 = field2\n"
    "        self.field3 = field3\n"
    "class class_4fields:\n"
    "    def __init__(self, field1, field2, field3, field4):\n"
    "        self.field1 = field1\n"
    "        self.field2 = field2\n"
    "        self.field3 = field3\n"
    "        self.field4 = field4\n"
    "class class_5fields:\n"
    "    def __init__(self, field1, field2, field3, field4, field5):\n"
    "        self.field1 = field1\n"
    "        self.field2 = field2\n"
    "        self.field3 = field3\n"
    "        self.field4 = field4\n"
    "        self.field5 = field5\n"
    "class class_6fields:\n"
    "    def __init__(self, field1, field2, field3, field4, field5, field6):\n"
    "        self.field1 = field1\n"
    "        self.field2 = field2\n"
    "        self.field3 = field3\n"
    "        self.field4 = field4\n"
    "        self.field5 = field5\n"
    "        self.field6 = field6\n"
   );

  PyObject* m = PyImport_AddModule("__main__");
  PyObject* class_2fields = PyObject_GetAttrString(m, "class_2fields");
  PyObject* class_3fields = PyObject_GetAttrString(m, "class_3fields");
  PyObject* class_4fields = PyObject_GetAttrString(m, "class_4fields");
  PyObject* class_5fields = PyObject_GetAttrString(m, "class_5fields");
  PyObject* class_6fields = PyObject_GetAttrString(m, "class_6fields");

  long arg_value = 0;
  PyObject* args;
  PyObject* arg1;
  PyObject* arg2;
  PyObject* arg3;
  PyObject* arg4;
  PyObject* arg5;
  PyObject* arg6;

  PyObject* current_instance;

  PyObject* collection_2fields;
  PyObject* collection_3fields;
  PyObject* collection_4fields;
  PyObject* collection_5fields;
  PyObject* collection_6fields = PyList_New(0);;

  for (int count_iterations = 0;  count_iterations < 1000;  count_iterations++) {
    collection_2fields = PyList_New(0);
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg1 = PyLong_FromLong(arg_value++);
      arg2 = PyLong_FromLong(arg_value++);
      args = PyTuple_Pack(2, arg1, arg2);
      Py_DECREF(arg1);
      Py_DECREF(arg2);

      current_instance = PyObject_Call(class_2fields, args, nullptr);
      Py_DECREF(args);

      PyList_Append(collection_2fields, current_instance);
      Py_DECREF(current_instance);
    }

    Py_DECREF(collection_6fields);

    collection_3fields = PyList_New(0);
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg1 = PyLong_FromLong(arg_value++);
      arg2 = PyLong_FromLong(arg_value++);
      arg3 = PyLong_FromLong(arg_value++);
      args = PyTuple_Pack(3, arg1, arg2, arg3);
      Py_DECREF(arg1);
      Py_DECREF(arg2);
      Py_DECREF(arg3);

      current_instance = PyObject_Call(class_3fields, args, nullptr);
      Py_DECREF(args);

      PyList_Append(collection_3fields, current_instance);
      Py_DECREF(current_instance);
    }

    Py_DECREF(collection_2fields);

    collection_4fields = PyList_New(0);
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg1 = PyLong_FromLong(arg_value++);
      arg2 = PyLong_FromLong(arg_value++);
      arg3 = PyLong_FromLong(arg_value++);
      arg4 = PyLong_FromLong(arg_value++);
      args = PyTuple_Pack(4, arg1, arg2, arg3, arg4);
      Py_DECREF(arg1);
      Py_DECREF(arg2);
      Py_DECREF(arg3);
      Py_DECREF(arg4);

      current_instance = PyObject_Call(class_4fields, args, nullptr);
      Py_DECREF(args);

      PyList_Append(collection_4fields, current_instance);
      Py_DECREF(current_instance);
    }

    Py_DECREF(collection_3fields);

    collection_5fields = PyList_New(0);
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg1 = PyLong_FromLong(arg_value++);
      arg2 = PyLong_FromLong(arg_value++);
      arg3 = PyLong_FromLong(arg_value++);
      arg4 = PyLong_FromLong(arg_value++);
      arg5 = PyLong_FromLong(arg_value++);
      args = PyTuple_Pack(5, arg1, arg2, arg3, arg4, arg5);
      Py_DECREF(arg1);
      Py_DECREF(arg2);
      Py_DECREF(arg3);
      Py_DECREF(arg4);
      Py_DECREF(arg5);

      current_instance = PyObject_Call(class_5fields, args, nullptr);
      Py_DECREF(args);

      PyList_Append(collection_5fields, current_instance);
      Py_DECREF(current_instance);
    }

    Py_DECREF(collection_4fields);

    collection_6fields = PyList_New(0);
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg1 = PyLong_FromLong(arg_value++);
      arg2 = PyLong_FromLong(arg_value++);
      arg3 = PyLong_FromLong(arg_value++);
      arg4 = PyLong_FromLong(arg_value++);
      arg5 = PyLong_FromLong(arg_value++);
      arg6 = PyLong_FromLong(arg_value++);
      args = PyTuple_Pack(6, arg1, arg2, arg3, arg4, arg5, arg6);
      Py_DECREF(arg1);
      Py_DECREF(arg2);
      Py_DECREF(arg3);
      Py_DECREF(arg4);
      Py_DECREF(arg5);
      Py_DECREF(arg6);

      current_instance = PyObject_Call(class_6fields, args, nullptr);
      Py_DECREF(args);

      PyList_Append(collection_6fields, current_instance);
      Py_DECREF(current_instance);
    }

    Py_DECREF(collection_5fields);
  }

  Py_DECREF(collection_6fields);

  printf("END\n");
}

PYBIND11_MODULE(testy, m) {
  m.def("run", &run);
}
