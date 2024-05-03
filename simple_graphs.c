#define PY_SSIZE_T_CLEAN
#define MAX_VERTICES 16

#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

static long number_of_vertices_helper(short vertices) {
    long verticesNumber = 0;
    while (vertices > 0) {
        if (vertices & 1)
            verticesNumber += 1;
        vertices >>= 1;
    }
    return verticesNumber;
}

static long is_edge_helper(short *edges, int u, int v) {
    return edges[u] & (1 << v);
}

typedef struct {
    PyObject_HEAD
    short vertices;
    short *edges;
} AdjacencyMatrix;

static PyTypeObject AdjacencyMatrixType;

static PyObject *AdjacencyMatrix_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    AdjacencyMatrix *self;
    self = (AdjacencyMatrix *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->vertices = 0x0000;
        self->edges = NULL;
    }
    return (PyObject *)self;
}

static int AdjacencyMatrix_init(AdjacencyMatrix *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = { "text", NULL };
    char *txt = "?";

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s", keywords, &txt)) {
        return -1;
    }

    self->edges = malloc(MAX_VERTICES * sizeof(short));
    for (int i = 0; i < MAX_VERTICES; i++) {
        self->edges[i] = 0x0000;
    }

    int vc = txt[0] - 63;
    int k = 0;
    int i = 1;
    int c = 0;
    for (int v = 1; v < vc; v++) {
        for (int u = 0; u < v; u++) {
            if (!k) {
                c = txt[i] - 63;
                i++;
                k = 6;
            }
            k--;
            if ((c & (1 << k)) != 0) {
                self->edges[u] |= (1 << v);
                self->edges[v] |= (1 << u);
            }
        }
    }

    self->vertices = (1 << vc) - 1;

    return 0;
}

static void AdjacencyMatrix_dealloc(AdjacencyMatrix *self) {
    free(self->edges);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *number_of_vertices(AdjacencyMatrix *self) {
    short v = self->vertices;
    long count = number_of_vertices_helper(v);
    return PyLong_FromLong(count);
}

static PyObject *vertices(AdjacencyMatrix *self) {
    PyObject *verts = PySet_New(NULL);
    if (!verts)
        return NULL;

    for (int i = 0; i < MAX_VERTICES; i++) {
        if (self->vertices & (1 << i)) {
            PyObject *v = PyLong_FromLong(i);
            PySet_Add(verts, v);
            Py_DECREF(v);
        }
    }

    return verts;
}

static PyObject *vertex_degree(AdjacencyMatrix *self, PyObject *args) {
    int vertex;
    if (args != NULL)
        PyArg_ParseTuple(args, "i", &vertex);

    long degree = 0;
    short vertexEdges = self->edges[vertex];
    while (vertexEdges > 0) {
        if (vertexEdges & 1)
            degree += 1;
        vertexEdges >>= 1;
    }
    return PyLong_FromLong(degree);
}

static PyObject *vertex_neighbors(AdjacencyMatrix *self, PyObject *args) {
    int vertex;
    if (args != NULL)
        PyArg_ParseTuple(args, "i", &vertex);

    PyObject *verts = PySet_New(NULL);
    if (!verts)
        return NULL;

    short vertexEdges = self->edges[vertex];
    for (int i = 0; i < MAX_VERTICES; i++) {
        if (vertexEdges & (1 << i)) {
            PyObject *v = PyLong_FromLong(i);
            PySet_Add(verts, v);
            Py_DECREF(v);
        }
    }

    return verts;
}

static PyObject *add_vertex(AdjacencyMatrix *self, PyObject *args) {
    int vertex;
    if (args != NULL)
        PyArg_ParseTuple(args, "i", &vertex);

    self->vertices |= (1 << vertex);
    return PyBool_FromLong(0);
}

static PyObject *delete_vertex(AdjacencyMatrix *self, PyObject *args) {
    int vertex;
    if (args != NULL)
        PyArg_ParseTuple(args, "i", &vertex);

    short mask = ~(1 << vertex);
    self->vertices &= mask;
    self->edges[vertex] = 0;

    for (int i = 0; i < MAX_VERTICES; i++) {
        self->edges[i] &= mask;
    }

    return PyBool_FromLong(1);
}

static PyObject *is_edge(AdjacencyMatrix *self, PyObject *args) {
    int u;
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &u, &v);
    }

    long ans = is_edge_helper(self->edges, u, v);

    return PyBool_FromLong(ans);
}

static PyObject *number_of_edges(AdjacencyMatrix *self) {
    long numberOfEdges = 0;
    for (int i = 0; i < MAX_VERTICES; i++) {
        short vertexEdges = self->edges[i];
        for (int j = 0; j < MAX_VERTICES; j++) {
            if (vertexEdges & (1 << j))
                numberOfEdges += 1;
        }
    }
    return PyLong_FromLong(numberOfEdges / 2);
}

static PyObject *edges(AdjacencyMatrix *self) {
    PyObject *edges = PySet_New(NULL);
    if (!edges)
        return NULL;

    for (int i = 0; i < MAX_VERTICES; i++) {
        short vertexEdges = self->edges[i];
        for (int j = 0; j < MAX_VERTICES; j++) {
            if (vertexEdges & (1 << j)) {
                PyObject *edge = PyTuple_New(2);
                PyTuple_SetItem(edge, 0, PyLong_FromLong(i < j ? i : j));
                PyTuple_SetItem(edge, 1, PyLong_FromLong(i < j ? j : i));
                PySet_Add(edges, edge);
                Py_DECREF(edge);
            }
        }
    }

    return edges;
}

static PyObject *add_edge(AdjacencyMatrix *self, PyObject *args) {
    int u;
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &u, &v);
    }

    if (v != u) {
        self->edges[u] |= (1 << v);
        self->edges[v] |= (1 << u);
    }

    return PyBool_FromLong(1);
}

static PyObject *delete_edge(AdjacencyMatrix *self, PyObject *args) {
    int u;
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &u, &v);
    }

    self->edges[u] &= ~(1 << v);
    self->edges[v] &= ~(1 << u);

    return PyBool_FromLong(1);
}

static PyObject *is_complete_bipartite(AdjacencyMatrix *self, PyObject *args) {
    long verticesNumber = number_of_vertices_helper(self->vertices);

    for (int u = 0; u < verticesNumber; u++) {
        for (int v = 0; v < verticesNumber; v++) {
            if (u != v) {
                if (is_edge_helper(self->edges, u, v)) {
                    if((is_edge_helper(self->edges, 0, u) && is_edge_helper(self->edges, 0, v)) || (!is_edge_helper(self->edges, 0, u) && !is_edge_helper(self->edges, 0, v))) {
                        return PyBool_FromLong(0);
                    }
                } else {
                    if((is_edge_helper(self->edges, 0, u) && !is_edge_helper(self->edges, 0, v)) || (!is_edge_helper(self->edges, 0, u) && is_edge_helper(self->edges, 0, v))) {
                        return PyBool_FromLong(0);
                    }
                }
            }
        }
    }

    return PyBool_FromLong(1);
}

static PyMemberDef AdjacencyMatrix_members[] = {
    {"vertices", T_SHORT, offsetof(AdjacencyMatrix, vertices), 0, PyDoc_STR("vertices of the graph")},
    {"edges", T_SHORT, offsetof(AdjacencyMatrix, edges), 0, PyDoc_STR("edges of the graph")},
    {NULL}};

static PyMethodDef AdjacencyMatrix_methods[] = {
    {"number_of_vertices", (PyCFunction)number_of_vertices, METH_NOARGS},
    {"vertices", (PyCFunction)vertices, METH_NOARGS},
    {"vertex_degree", (PyCFunction)vertex_degree, METH_VARARGS},
    {"vertex_neighbors", (PyCFunction)vertex_neighbors, METH_VARARGS},
    {"add_vertex", (PyCFunction)add_vertex, METH_VARARGS},
    {"delete_vertex", (PyCFunction)delete_vertex, METH_VARARGS},
    {"is_edge", (PyCFunction)is_edge, METH_VARARGS},
    {"number_of_edges", (PyCFunction)number_of_edges, METH_NOARGS},
    {"edges", (PyCFunction)edges, METH_NOARGS},
    {"add_edge", (PyCFunction)add_edge, METH_VARARGS},
    {"delete_edge", (PyCFunction)delete_edge, METH_VARARGS},
    {"is_complete_bipartite", (PyCFunction)is_complete_bipartite, METH_VARARGS},
    {NULL, NULL}};

static PyTypeObject AdjacencyMatrixType = {
    PyVarObject_HEAD_INIT(NULL, 0) "simple_graphs.AdjacencyMatrix",
    sizeof(AdjacencyMatrix),
    0,
    (destructor)AdjacencyMatrix_dealloc, /* tp_dealloc */
    0,                                   /* tp_vectorcall_offset */
    0,                                   /* tp_getattr */
    0,                                   /* tp_setattr */
    0,                                   /* tp_as_async */
    0,                                   /* tp_repr */
    0,                                   /* tp_as_number */
    0,                                   /* tp_as_sequence */
    0,                                   /* tp_as_mapping */
    0,                                   /* tp_hash */
    0,                                   /* tp_call */
    0,                                   /* tp_str */
    0,                                   /* tp_getattro */
    0,                                   /* tp_setattro */
    0,                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                  /* tp_flags */
    0,                                   /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    AdjacencyMatrix_methods,             /* tp_methods */
    AdjacencyMatrix_members,             /* tp_members */
    0,                                   /* tp_getset */
    0,                                   /* tp_base */
    0,                                   /* tp_dict */
    0,                                   /* tp_descr_get */
    0,                                   /* tp_descr_set */
    0,                                   /* tp_dictoffset */
    (initproc)AdjacencyMatrix_init,      /* tp_init */
    0,                                   /* tp_alloc */
    AdjacencyMatrix_new,                 /* tp_new */
};

static struct PyModuleDef graphmodule = {
    PyModuleDef_HEAD_INIT,
    "simple_graphs",
    NULL,
    -1};

PyMODINIT_FUNC PyInit_simple_graphs(void) {
    PyObject *m;
    if (PyType_Ready(&AdjacencyMatrixType) < 0)
        return NULL;

    m = PyModule_Create(&graphmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&AdjacencyMatrixType);
    if (PyModule_AddObject(m, "AdjacencyMatrix", (PyObject *)&AdjacencyMatrixType) < 0) {
        Py_DECREF(&AdjacencyMatrixType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
