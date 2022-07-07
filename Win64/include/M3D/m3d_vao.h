#ifndef M3D_VAO
#define M3D_VAO

class VertexArrayObject
{
public:

	unsigned int objectID;

	VertexArrayObject();

	inline void fBind() const {		glBindVertexArray(objectID);	}

	void fAddArrayBuffer(unsigned int*, unsigned int, const void*, unsigned int, unsigned int, unsigned int*);

	void fAddElementBuffer(unsigned int*, unsigned int, const void*);

};

#endif