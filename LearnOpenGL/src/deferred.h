#ifndef DEFERRED_H
#define DEFERRED_H

#include <glad/glad.h> 
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "object.h"
#include "light.h"
#include "ssao.h"
#include "model.h"

class Deferred {
public:
	Deferred(bool enableSSAO=false);
	void DrawGbuffer(vector<Object> objects, glm::mat4 view, glm::mat4 projection);
	void DrawGbuffer(Model model, glm::mat4 mModel, glm::mat4 view, glm::mat4 projection);
	void DrawDeferred(vector<std::shared_ptr<Light>>lights, unsigned int FBO, glm::vec3 viewPos, const glm::mat4& view, const glm::mat4& projection);
	unsigned int gFBO;
	unsigned int gPositionDepth, gNormal, gAlbedoSpec;
	unsigned int gDepth;
	SSAO ssao = SSAO(gPositionDepth, gNormal);

private:
	unsigned int SCR_WIDTH = 800;
	unsigned int SCR_HEIGHT = 600;
	void GetFramebuffer();
	void GetVertexArray();
	unsigned int frameVAO;
	unsigned int frameVBO;
	Shader gbufferShader = Shader("res/shader/gbufferVertex.shader", "res/shader/gbufferFragment.shader");
	Shader deferredDirShader = Shader("res/shader/deferredDirVertex.shader", "res/shader/deferredDirFragment.shader");
	Shader deferredDotShader = Shader("res/shader/deferredDotVertex.shader", "res/shader/deferredDotFragment.shader");
	//Shader deferredSpotShader = Shader("res/shader/deferredVertex.shader", "res/shader/deferredSpotFragment.shader");
	Shader ssaoDeferredDirShader = Shader("res/shader/deferredDirVertex.shader", "res/shader/ssaoDeferredDirFragment.shader");
	Shader ssaoDeferredDotShader = Shader("res/shader/deferredDotVertex.shader", "res/shader/ssaoDeferredDotFragment.shader");
	void DrawByLight(std::shared_ptr<Light> light, const glm::vec3& viewPos, const glm::mat4& view, const glm::mat4 projection, Shader dotShader, Shader dirShader);
	bool EnableSSAO;
};

Deferred::Deferred(bool enableSSAO)
	:EnableSSAO(enableSSAO)
{
	GetFramebuffer();
	GetVertexArray();
	ssao = SSAO(gPositionDepth, gNormal);
}

void Deferred::GetFramebuffer()
{
	glGenFramebuffers(1, &gFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

	glGenTextures(1, &gPositionDepth);
	glBindTexture(GL_TEXTURE_2D, gPositionDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionDepth, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	unsigned int defAttcahments[3] = { GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, defAttcahments);

	glGenRenderbuffers(1, &gDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Deferred::GetVertexArray()
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

void Deferred::DrawGbuffer(vector<Object> objects, glm::mat4 view, glm::mat4 projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	for (unsigned int i = 0; i != objects.size(); i++)
	{
		gbufferShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, objects[i].texture_diffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, objects[i].texture_specular);

		gbufferShader.setMat4("view", glm::value_ptr(view));
		gbufferShader.setMat4("projection", glm::value_ptr(projection));
		gbufferShader.setMat4("model", glm::value_ptr(objects[i].model));

		gbufferShader.setInt("texture_diffuse", 0);
		gbufferShader.setInt("texture_specular", 1);

		glBindVertexArray(objects[i].VAO);
		glDrawElements(GL_TRIANGLES, objects[i].Count, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Deferred::DrawGbuffer(Model model, glm::mat4 mModel, glm::mat4 view, glm::mat4 projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	for (unsigned int i = 0; i !=model.meshes.size(); i++)
	{
		gbufferShader.use();

		bool diff = false;
		bool spec = false;

		for (int j = 0; j != model.meshes[i].textures.size(); j++)
		{
			string name = model.meshes[i].textures[j].type;
			if (name == "texture_diffuse" && diff == false)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, model.meshes[i].textures[j].id);
				diff = true;
			}
			if (name == "texture_specular" && spec == false)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, model.meshes[i].textures[j].id);
				spec = true;
			}
		}

		gbufferShader.setMat4("view", glm::value_ptr(view));
		gbufferShader.setMat4("projection", glm::value_ptr(projection));
		gbufferShader.setMat4("model", glm::value_ptr(mModel));

		gbufferShader.setInt("texture_diffuse", 0);
		gbufferShader.setInt("texture_specular", 1);

		glBindVertexArray(model.meshes[i].deferVAO);
		glDrawElements(GL_TRIANGLES, model.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Deferred::DrawDeferred(vector<std::shared_ptr<Light>>lights, unsigned int FBO, glm::vec3 viewPos, const glm::mat4& view, const glm::mat4& projection)
{
	if (EnableSSAO)
	{
		ssao.Draw(projection);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);

	glDisable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	for (unsigned int i = 0; i != lights.size(); i++)
	{
		if (!EnableSSAO)
			DrawByLight(lights[i], viewPos, view, projection,deferredDotShader,deferredDirShader);
		else
			DrawByLight(lights[i], viewPos, view, projection,ssaoDeferredDotShader,ssaoDeferredDirShader);
		
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
}

void Deferred::DrawByLight(std::shared_ptr<Light> light, const glm::vec3 &viewPos, const glm::mat4 &view, const glm::mat4 projection, Shader dotShader, Shader dirShader)
{
	if (light->GetType() == Dot)
	{
		dotShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		dotShader.setInt("gPositionDepth", 0);
		dotShader.setInt("gNormal", 1);
		dotShader.setInt("gAlbedoSpec", 2);
		dotShader.setVec3("viewPos", viewPos);
		dotShader.setFloat("shininess", 36.0f);

		if (EnableSSAO)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssao.GetSSAO());
			dotShader.setInt("ssao", 3);
		}

		LightInfo info;
		light->GetLightInfo(info);
		dotShader.setVec3("pointlight.position", info.Position);
		dotShader.setVec3("pointlight.ambient", info.Ambient);
		dotShader.setVec3("pointlight.diffuse", info.Diffuse);
		dotShader.setVec3("pointlight.specular", info.Specular);
		dotShader.setFloat("pointlight.constant", info.Constant);
		dotShader.setFloat("pointlight.linear", info.Linear);
		dotShader.setFloat("pointlight.quadratic", info.Quadratic);

		unsigned int SphereVAO;
		unsigned int SphereCount;
		glm::mat4 SphereModel;
		light->GetSphere(SphereVAO, SphereCount, SphereModel);
		dotShader.setMat4("model", glm::value_ptr(SphereModel));
		dotShader.setMat4("view", glm::value_ptr(view));
		dotShader.setMat4("projection", glm::value_ptr(projection));

		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereCount,GL_UNSIGNED_INT,0);
	}
	else if (light->GetType() == Dir)
	{
		dirShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		dirShader.setInt("gPositionDepth", 0);
		dirShader.setInt("gNormal", 1);
		dirShader.setInt("gAlbedoSpec", 2);
		dirShader.setVec3("viewPos", viewPos);
		dirShader.setFloat("shininess", 36.0f);

		if (EnableSSAO)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssao.GetSSAO());
			dirShader.setInt("ssao", 3);
		}

		LightInfo info;
		light->GetLightInfo(info);

		dirShader.setVec3("dirlight.direction", info.Direction);
		dirShader.setVec3("dirlight.ambient", info.Ambient);
		dirShader.setVec3("dirlight.diffuse", info.Diffuse);
		dirShader.setVec3("dirlight.specular", info.Specular);

		dirShader.setMat4("view", glm::value_ptr(view));

		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0,6);
	}

}

#endif
