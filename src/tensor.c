#include <Python.h>
#include <stddef.h>
#include "structmember.h"

typedef struct{
	PyObject_HEAD
	unsigned len;
	float *arr;
}tensor;

static PyObject *tensor_new(PyTypeObject *type,PyObject *args,PyObject *kwds){
	tensor *obj;
	obj = (tensor*)type->tp_alloc(type,0);
	
	return (PyObject*)obj;
}

static int tensor_init(tensor *self,PyObject *args,PyObject *kwds){
	int len;
	if(!PyArg_ParseTuple(args,"i",&len)){
		return -1;
	}
	self->len = len;
	self->arr = malloc(len * sizeof(float));
	
	if(self->arr == NULL){
		return -1;
	}

	return 0;
}

static void tensor_dealloc(tensor *self){
	if(self->arr != NULL){
		free(self->arr);
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *tensor_str(tensor *self){
	const int BLOCK_SIZE = 64;
	unsigned cap = BLOCK_SIZE;
	char *ans = malloc(cap);

	int aux = sprintf(ans,"array([");
	for(int i=0;i<self->len;++i){
		aux += sprintf(ans + aux,"%f,",self->arr[i]);

		if(cap - aux <= BLOCK_SIZE / 30){
			cap += BLOCK_SIZE;
			ans = realloc(ans,cap);
		}
	}
	sprintf(ans + aux - 1,"])");
	PyObject *ansO = Py_BuildValue("s",ans);
	free(ans);
	return ansO;
}

static int tensor_len(tensor *self){
	return self->len;
}

static PyMemberDef tensor_members[] = {
	{"len",T_INT,offsetof(tensor,len),0,"current length"},
	{NULL}
};

static PyObject *tensor_getitem(tensor *self,PyObject *key){
	if(!PyLong_Check(key)){
		PyErr_SetString(PyExc_TypeError,"Subscript must be an integer!");
		return NULL;
	}

	int pos = PyLong_AsLong(key);

	if(pos > self->len){
		PyErr_SetString(PyExc_TypeError,"Index out of bounds!");
		return NULL;
	}

	return Py_BuildValue("f",self->arr[pos]);
}

static int tensor_setitem(tensor *self,PyObject *key,PyObject *val){
	int _key;
	float _val;

	if(!PyLong_Check(key)){
		PyErr_SetString(PyExc_TypeError,"Subscript must be an integer!");
		return -1;
	}

	if(PyFloat_Check(val)){
		_val = PyFloat_AsDouble(val);
	}
	else if(PyLong_Check(val)){
		_val = (float)PyLong_AsLong(val);
	}
	else{
		PyErr_SetString(PyExc_TypeError,"Invalid value!");
		return -1;
	}

	_key = PyLong_AsLong(key);

	if(_key > self->len){
		PyErr_SetString(PyExc_TypeError,"Index out of bounds!");
		return -1;
	}

	self->arr[_key] = _val;
	
	return 0;
}

static PyMethodDef tensor_methods[] = {
	{"name",(PyObject* (*)(PyObject*,PyObject*))tensor_str,METH_NOARGS,"name of tensor object"},
	{NULL}
};

static PySequenceMethods tensor_sequence = {
	.sq_length = (Py_ssize_t (*)(PyObject*))tensor_len,
};

static PyMappingMethods tensor_mapping = {
	.mp_subscript = (PyObject * (*)(PyObject *,PyObject *))tensor_getitem,
	.mp_ass_subscript = (int (*)(PyObject *,PyObject *,PyObject *))tensor_setitem,
};

static PyTypeObject tensor_type = {
	.ob_base = PyVarObject_HEAD_INIT(NULL,0)
	.tp_name = "tensor.tensor",
	.tp_basicsize = sizeof(tensor),
	.tp_new = tensor_new,
	.tp_init = (int (*)(PyObject *, PyObject *, PyObject *))tensor_init,
	.tp_dealloc = (void (*)(PyObject *))tensor_dealloc,
	.tp_doc = "tensor class type",
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_members = tensor_members,
	.tp_methods = tensor_methods,
	.tp_str = (PyObject * (*)(PyObject *))tensor_str,
	.tp_as_sequence = &tensor_sequence,
	.tp_as_mapping = &tensor_mapping,
};

static PyModuleDef tensor_module = {
	PyModuleDef_HEAD_INIT,
	"tensor",
	"tensor module",
	-1
};

PyObject *PyInit_tensor(){
	PyObject *mod;

	if(PyType_Ready(&tensor_type) < 0){
		return NULL;
	}

	mod = PyModule_Create(&tensor_module);
	if(mod == NULL){
		return NULL;
	}
	PyModule_AddObject(mod,"tensor",(PyObject*)&tensor_type);

	return mod;
}