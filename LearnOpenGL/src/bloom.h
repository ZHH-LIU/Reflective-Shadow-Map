#ifndef BLOOM_H
#define BLOOM_H

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map> 

class Bloom {
public:
	Bloom();
	void DrawBloom(unsigned int colorBuffer);
	unsigned int bloomColorBuffer;
	
private:
	void GetFramebuffer();
	void GetVertexArray();
	void GetBlurFramebuffer();
	void GetResultFramebuffer();
	Shader twobufferShader = Shader("res/shader/twobufferVertex.shader", "res/shader/twobufferFragment.shader");
	Shader blurShader = Shader("res/shader/blurVertex.shader", "res/shader/blurFragment.shader");
	Shader bloomShader = Shader("res/shader/bloomVertex.shader", "res/shader/bloomFragment.shader");
	void DrawTwoBuffer(unsigned int colorBuffer);
	void DrawGaussBlur();
	unsigned int bloomFBO;
	unsigned int colorBuffers[2];
	unsigned int blurFBO[2];
	unsigned int blurColorBuffers[2];
	unsigned int resultFBO;
	unsigned int WIDTH = 800;
	unsigned int HEIGHT = 600;
	unsigned int frameVAO;
	unsigned int frameVBO;
};


Bloom::Bloom()
{
	GetFramebuffer();
	GetVertexArray();
	GetBlurFramebuffer();
	GetResultFramebuffer();
}

void Bloom::GetFramebuffer()
{
	glGenFramebuffers(1, &bloomFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);

	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i != 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::GetVertexArray()
{
	float frameVertices[] =
	{
		-1.0, -1.0,  0.0, 0.0,
		 1.0, -1.0,  1.0, 0.0,
		 1.0,  1.0,  1.0, 1.0,
		 1.0,  1.0,  1.0, 1.0,
		-1.0,  1.0,  0.0, 1.0,
		-1.0, -1.0,  0.0, 0.0
	};

	glGenBuffers(1, &frameVBO);
	glGenVertexArrays(1, &frameVAO);

	glBindVertexArray(frameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frameVertices), frameVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Bloom::GetBlurFramebuffer()
{
	glGenFramebuffers(2, blurFBO);
	glGenTextures(2, blurColorBuffers);
	
	for (unsigned int i = 0; i != 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);

		glBindTexture(GL_TEXTURE_2D, blurColorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffers[i], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::GetResultFramebuffer()
{
	glGenFramebuffers(1, &resultFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, resultFBO);

	glGenTextures(1, &bloomColorBuffer);

	glBindTexture(GL_TEXTURE_2D, bloomColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomColorBuffer, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::DrawTwoBuffer(unsigned int colorBuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, WIDTH, HEIGHT);

	twobufferShader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	twobufferShader.setInt("texColorBuffer", 0);
	twobufferShader.setFloat("exposure", 1.0f);

	glBindVertexArray(frameVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Bloom::DrawGaussBlur()
{
	bool horizontal = true;
	bool first = true;

	unsigned int times = 10;
	for (unsigned int i = 0; i != times; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);

		blurShader.use();

		unsigned int texture = first ? colorBuffers[1] : blurColorBuffers[!horizontal];
		glBindTexture(GL_TEXTURE_2D, texture);

		blurShader.setBool("horizontal", horizontal);

		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		if(first)
			first = false;
		horizontal = !horizontal;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::DrawBloom(unsigned int colorBuffer)
{
	DrawTwoBuffer(colorBuffer);
	DrawGaussBlur();

	glBindFramebuffer(GL_FRAMEBUFFER, resultFBO);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, WIDTH, HEIGHT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blurColorBuffers[0]);

	bloomShader.use();
	bloomShader.setInt("texColorBuffer", 0);
	bloomShader.setInt("blurColorBuffer", 1);
	bloomShader.setFloat("exposure", 1.0f);
	glBindVertexArray(frameVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif
