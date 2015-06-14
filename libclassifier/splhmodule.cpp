#include <Python.h>
#include <numpy/arrayobject.h>
#include <iostream>
#include "Image.h"
#include <fstream>

static PyObject* SplhError;

static std::vector<Hasher> hashers;
static HashBase hashBase(4, 16, 1, 4);

typedef uint64_t T;

static PyObject* splh_classify(PyObject *self, PyObject * args)
{
	PyObject *frame;

	if (!PyArg_ParseTuple(args, "O", &frame))
		return NULL;

	double *tab;

	int typenum = NPY_DOUBLE;
	PyArray_Descr *descr = PyArray_DescrFromType(typenum);
	npy_intp dims[1];

	if (PyArray_AsCArray(&frame, (void**)&tab, dims, 1, descr) < 0)
		return NULL;

	T hash = 0;

	if (dims[0] != 32*32*3)
		return Py_BuildValue("(i,i)", (int)-1, (int)666);

	for (unsigned int n=0;n<hashers.size();++n)
	{
		double b = hashers[n].bias;
		int l = 0;
		for (int i=0;i<3;++i)
		for (int y=0;y<32;++y)
		for (int x=0;x<32;++x)
			b += hashers[n].v[l++] * tab[y*32*3+x*3+2-i] / 255.0;
		
		if (b > 0)
			hash |= T(1) << n;
	}

	HashBase::Cls result = hashBase.Find(hash);

	if (result.cls == 255)
		result.cls = -1;

	//std::cout << ":: dbresult : " << result.cls << " " << result.hamming << std::endl;

	free(tab);

	//return PyLong_FromLong(result.cls);
	return Py_BuildValue("(i,i)", (int)result.cls, (int)result.hamming);
}

PyMODINIT_FUNC initsplh(void)
{
	PyObject *m;
	static PyMethodDef SplhMethods[] = {
		{"classify", splh_classify, METH_VARARGS, "Classification."},
		{NULL, NULL, 0, NULL}
	};
	m = Py_InitModule("splh", SplhMethods);
	if ( m == NULL )
		return;

	SplhError = PyErr_NewException((char*)"splh.error", NULL, NULL);
	Py_INCREF(SplhError);
	PyModule_AddObject(m, "error", SplhError);

	import_array();

	{
		std::ifstream file("class.txt");
		hashers = LoadHashers(file);
	}
	{
		std::ifstream file("img_base.txt");
		hashBase.Load(file);
	}
}

int main(int argc, char *argv[])
{
	Py_SetProgramName(argv[0]);
	Py_Initialize();
	initsplh();

	return 0;
}

