#include <Python.h>
#include <tcl.h>
#include <tclCompile.h>

static Tcl_Interp *interp;
extern const void *TclGetInstructionTable(void);

static PyObject *
tcldis_test(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "", kwlist))
		return NULL;

	Tcl_Obj *tObj = Tcl_NewObj();
	Tcl_AppendStringsToObj(tObj, "puts 1", NULL);

	const Tcl_ObjType *bct = Tcl_GetObjType("bytecode");
	if (Tcl_ConvertToType(interp, tObj, bct) != TCL_OK)
		return PyLong_FromLong(1);

	return PyLong_FromLong(0);
}

static PyObject *
tcldis_inst_table(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "", kwlist))
		return NULL;

	const InstructionDesc *insts =
		(const InstructionDesc *)TclGetInstructionTable();

	PyObject *pInsts = PyList_New(0);
	if (pInsts == NULL)
		return NULL;

	InstructionDesc inst;
	int i = 0, opIdx;
	PyObject *pInst = NULL;
	PyObject *pInstName = NULL;
	PyObject *pInstNumBytes = NULL;
	PyObject *pInstStackEffect = NULL;
	PyObject *pInstOperands = NULL, *pInstOperand = NULL;
	while (1) {
		inst = insts[i];
		if (inst.name == NULL)
			break;

		pInst = PyDict_New();
		if (pInst == NULL)
			goto err;

		pInstName = PyString_FromString(inst.name);
		if (pInstName == NULL)
			goto err;
		if (PyDict_SetItemString(pInst, "name", pInstName) != 0)
			goto err;
		Py_CLEAR(pInstName);

		pInstNumBytes = PyInt_FromLong(inst.numBytes);
		if (pInstNumBytes == NULL)
			goto err;
		if (PyDict_SetItemString(pInst, "num_bytes", pInstNumBytes) != 0)
			goto err;
		Py_CLEAR(pInstNumBytes);

		if (inst.stackEffect == INT_MIN) {
			pInstStackEffect = PyString_FromString("op1");
		} else {
			pInstStackEffect = PyInt_FromLong(inst.stackEffect);
		}
		if (pInstStackEffect == NULL)
			goto err;
		if (PyDict_SetItemString(pInst, "stack_effect", pInstStackEffect) != 0)
			goto err;
		Py_CLEAR(pInstStackEffect);

		pInstOperands = PyList_New(0);
		if (pInstOperands == NULL)
			goto err;
		for (opIdx = 0; opIdx < inst.numOperands; opIdx++) {
			pInstOperand = PyInt_FromLong(inst.opTypes[opIdx]);
			if (pInstOperand == NULL)
				goto err;
			if (PyList_Append(pInstOperands, pInstOperand) != 0)
				goto err;
			Py_CLEAR(pInstOperand);
		}
		if (PyDict_SetItemString(pInst, "operands", pInstOperands) != 0)
			goto err;
		Py_CLEAR(pInstOperands);

		if (PyList_Append(pInsts, pInst) != 0)
			goto err;

		i++;
	}

	return pInsts;

err:
	Py_XDECREF(pInsts);
	Py_XDECREF(pInst);
	Py_XDECREF(pInstName);
	Py_XDECREF(pInstNumBytes);
	Py_XDECREF(pInstStackEffect);
	Py_XDECREF(pInstOperands); Py_XDECREF(pInstOperand);
	return NULL;
}

static PyMethodDef TclDisMethods[] = {
	{"test",  (PyCFunction)tcldis_test,
		METH_VARARGS | METH_KEYWORDS, "test"},
	{"inst_table",  (PyCFunction)tcldis_inst_table,
		METH_VARARGS | METH_KEYWORDS, "inst_table"},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

/* Module name is _tcldis */
PyMODINIT_FUNC
init_tcldis(void)
{
	interp = Tcl_CreateInterp();

	PyObject *m = Py_InitModule("_tcldis", TclDisMethods);
	if (m == NULL)
		return;
}


