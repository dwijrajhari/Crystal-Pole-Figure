#include <M3D/m3d_basics.h>
#include <M3D/m3d_vao.h>

VertexArrayObject::VertexArrayObject() 
{
	glGenVertexArrays(1, &objectID);
	glBindVertexArray(objectID);
}

void VertexArrayObject::fAddArrayBuffer(unsigned int* bufferID, unsigned int dataSize, const void* data, unsigned int vertexSize, unsigned int attribCount, unsigned int* layout)
{
	glGenBuffers(1, bufferID);
	glBindVertexArray(objectID);

	glBindBuffer(GL_ARRAY_BUFFER, *bufferID);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

	unsigned int count = 0;
	for (unsigned char i = 0; i < attribCount; ++i)
	{
		glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, vertexSize, (void*)(count * sizeof(float)));
		glEnableVertexAttribArray(i);

		count += layout[i];
	}
}

void VertexArrayObject::fAddElementBuffer(unsigned int* bufferID, unsigned int dataSize, const void* data)
{
	glGenBuffers(1, bufferID);
	glBindVertexArray(objectID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
}



