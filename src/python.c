#include <python2.7/Python.h>
#include "python2.7/structmember.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "rpg.h"
#include "sprite.h"
#include "python.h"

#ifndef Py_TYPE
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

map_t map;
point_t camera;

PyObject *pName, *pModule, *pDict, *pFunc;

typedef struct {
	PyObject_HEAD
	sprite_t sprite;
} rpg_SpriteObject;

static PyObject*
rpg_SpriteObject_new(PyTypeObject *type, /*@unused@*/PyObject *args, /*@unused@*/PyObject *kwds)
{
	rpg_SpriteObject* self;
	self = (rpg_SpriteObject*)type->tp_alloc(type, 0);
	return (PyObject*)self;
}

static void
rpg_SpriteObject_dealloc(rpg_SpriteObject* self)
{
	if (freeSprite(&self->sprite) < 0)
		return;
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static int
rpg_SpriteObject_init(rpg_SpriteObject* self, PyObject*args, PyObject *kwds)
{
	char* path;
	int width  = TILE_SIZE;
	int height = TILE_SIZE;

	static char *kwlist[] = {"path", "width", "height", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|ii", kwlist, &path, &width, &height))
		return -1;
	
	if(loadSprite(path, &self->sprite, width, height) != 0)
		return -2;

	self->sprite.tile_x=-1;
	self->sprite.tile_y=-1;
	self->sprite.animation=RIGHT;
	self->sprite.screen_x= TILE_SIZE * self->sprite.tile_x;
	self->sprite.screen_y= TILE_SIZE * self->sprite.tile_y;

	return 0;
}

static PyObject*
rpg_SpriteObject_Move(rpg_SpriteObject* self, PyObject* args)
{
	int x, y;
	
	if (!PyArg_ParseTuple(args, "ii", &x, &y))
		return NULL;
	
	moveSprite(&self->sprite, x, y);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
rpg_SpriteObject_SetTile(rpg_SpriteObject* self, PyObject* args)
{
	int x, y;
		
	if (!PyArg_ParseTuple(args, "ii", &x, &y))
		return NULL;
	
	//if (&self->sprite==NULL)
		//return NULL;
	
	if (x < 0 || y < 0)
	{
		PyErr_SetString(PyExc_ValueError, "Tile coordinates must be 0 or greater.");
		return NULL;
	}

	if (map.tiles[x][y].occupied==1 || map.tiles[x][y].blocked==1)
	{
		PyErr_SetString(PyExc_ValueError, "Tile is in use or is blocked.");
		return NULL;
	}
	
	self->sprite.tile_x=x;
	self->sprite.tile_y=y;
	self->sprite.screen_x= TILE_SIZE * self->sprite.tile_x;
	self->sprite.screen_y= TILE_SIZE * self->sprite.tile_y;

	map.tiles[x][y].occupied=1;

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
rpg_SpriteObject_SetAI(rpg_SpriteObject* self, PyObject* args)
{
	char* flag;
	//PyFunctionObject* temp;

	if (!PyArg_ParseTuple(args, "s", &flag))
		return NULL;
	
	if (self->sprite.ai != NULL)
		self->sprite.ai(&self->sprite, CLEAR);

	if (strcmp(flag, "NULL") == 0)
		self->sprite.ai=NULL;
		
	if (strcmp(flag, "RANDOM") == 0)
	{
		self->sprite.ai=random_ai;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// Seperate just for pyai, takes list of functions for each flag
static PyObject*
rpg_SpriteObject_SetPythonAI(rpg_SpriteObject* self, PyObject* args)
{
	// need to create args
	//rpg_SpriteObject_SetAI(self, 
}

static PyMemberDef rpg_SpriteObject_members[] = {
	{"animation", T_INT, offsetof(rpg_SpriteObject, sprite.animation), 0,
	 "animation direction"},
	{NULL}
};

static PyMethodDef rpg_SpriteObject_methods[] = {
	{"SetTile", (PyCFunction)rpg_SpriteObject_SetTile, METH_VARARGS,
	 "Set the x and y tile position of the sprite."
	},
	{"SetAI", (PyCFunction)rpg_SpriteObject_SetAI, METH_VARARGS,
	 "Set the AI function of the sprite. Can be NULL, or RANDOM"
	},
	{"Move", (PyCFunction)rpg_SpriteObject_Move, METH_VARARGS,
	 "Move the sprite relatively by x, y pixels"},
	{NULL}
};

static PyTypeObject rpg_SpriteType = {
	PyObject_HEAD_INIT(NULL)
	0,                         		/*ob_size*/
	"rpg.Sprite",             		/*tp_name*/
	sizeof(rpg_SpriteObject),  		/*tp_basicsize*/
	0,                         		/*tp_itemsize*/
	(destructor)rpg_SpriteObject_dealloc,	/*tp_dealloc*/
	0,                         		/*tp_print*/
	0,                         		/*tp_getattr*/
	0,                         		/*tp_setattr*/
	0,                         		/*tp_compare*/
	0,                         		/*tp_repr*/
	0,                         		/*tp_as_number*/
	0,                         		/*tp_as_sequence*/
	0,                         		/*tp_as_mapping*/
	0,                         		/*tp_hash */
	0,                         		/*tp_call*/
	0,                         		/*tp_str*/
	0,                         		/*tp_getattro*/
	0,                         		/*tp_setattro*/
	0,                         		/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        		/*tp_flags*/
	"Sprite objects",          		/*tp_doc*/
	0,					/*tp_traverse*/
	0,					/*tp_clear*/
	0,					/*tp_richcompare*/
	0,					/*tp_weaklistoffset*/
	0,					/*tp_iter*/
	0,					/*tp_iternext*/
	rpg_SpriteObject_methods,		/*tp_methods*/
	rpg_SpriteObject_members,		/*tp_members*/
	0,					/*tp_getset*/
	0,					/*tp_base*/
	0,					/*tp_dict*/
	0,					/*tp_descr_get*/
	0,					/*tp_descr_set*/
	0,					/*tp_dictoffset*/
	(initproc)rpg_SpriteObject_init,	/*tp_init*/
	0,					/*tp_alloc*/
	rpg_SpriteObject_new,			/*tp_new*/
};

static PyMethodDef rpgMethods[] = {
	{"test", PY_Test, METH_VARARGS,
	 "Prints \"Hello, World!\""},
	{NULL, NULL, 0, NULL}
};

PyModINIT_FUNC
initrpg(void)
{
	if (PyType_Ready(&rpg_SpriteType) < 0)
		return;

#if PY_MAJOR_VERSION >= 3
	static struct PyModuleDef moduledef = {
			PyModuleDef_HEAD_INIT,
			"rpg", /* m_name */
			"Embedded Module for RPG scripts",                /* m_doc */
			-1,                  /* m_size */
			rpgMethods,          /* m_methods */
			NULL,                /* m_reload */
			NULL,                /* m_traverse */
			NULL,                /* m_clear */
			NULL,                /* m_free */
	};
#endif

#if PY_MAJOR_VERSION >= 3
	static struct PyModuleDef submoduledef = {
			PyModuleDef_HEAD_INIT,
			"rpg.constants", /* m_name */
			"Constant value submodule for RPG scripts",                /* m_doc */
			-1,                  /* m_size */
			NULL,          /* m_methods */
			NULL,                /* m_reload */
			NULL,                /* m_traverse */
			NULL,                /* m_clear */
			NULL,                /* m_free */
	};
#endif

#if PY_MAJOR_VERSION >= 3
	pModule = PyModule_Create(&moduledef);
#else
	pModule = Py_InitModule3("rpg", rpgMethods, "Embedded Module for RPG scripts");
#endif
	if (pModule == NULL)
		return;

	Py_INCREF(&rpg_SpriteType);
	PyModule_AddObject(pModule, "Sprite", (PyObject *)&rpg_SpriteType);

#if PY_MAJOR_VERSION >= 3
	PyObject* constants = PyModule_Create(&moduledef);
#else
	PyObject* constants = Py_InitModule3("rpg.constants", NULL, "Constant value submodule for RPG scripts");
#endif
	if (constants == NULL)
		return;

	PyModule_AddIntConstant(constants, "UP", UP);
	PyModule_AddIntConstant(constants, "DOWN", DOWN);
	PyModule_AddIntConstant(constants, "LEFT", LEFT);
	PyModule_AddIntConstant(constants, "RIGHT", RIGHT);

	PyModule_AddIntConstant(constants, "TILE_SIZE", TILE_SIZE);

	PyModule_AddObject(pModule, "rpg.constants", constants);
}

static PyObject*
PY_Test(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	return Py_BuildValue("s", "Hello, World!\n");
}
