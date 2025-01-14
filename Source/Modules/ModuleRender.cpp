#include "StdAfx.h"

#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModulePlayer.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleNavigation.h"

#include "Camera/Camera.h"
#include "Camera/CameraGameObject.h"

#include "Cubemap/Cubemap.h"

#include "Components/ComponentDirLight.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentParticleSystem.h"
#include "Components/ComponentSkybox.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentLine.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentRigidBody.h"

#include "DataModels/Resources/ResourceMaterial.h"
#include "DataModels/Batch/BatchManager.h"
#include "DataModels/GBuffer/GBuffer.h"

#include "DataStructures/Quadtree.h"

#include "FileSystem/ModuleResources.h"
#include "FileSystem/ModuleFileSystem.h"

#include "Program/Program.h"

#include "Scene/Scene.h"

#ifdef DEBUG
	#include "optick.h"
#endif // DEBUG

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
}

void __stdcall OurOpenGLErrorFunction(GLenum source,
									  GLenum type,
									  GLuint id,
									  GLenum severity,
									  GLsizei length,
									  const GLchar* message,
									  const void* userParam)
{
	const char *tmpSource = "", *tmpType = "", *tmpSeverity = "";

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
			tmpSource = "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			tmpSource = "Window System";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			tmpSource = "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			tmpSource = "Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			tmpSource = "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			tmpSource = "Other";
			break;
	};

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
			tmpType = "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			tmpType = "Deprecated Behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			tmpType = "Undefined Behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			tmpType = "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			tmpType = "Performance";
			break;
		case GL_DEBUG_TYPE_MARKER:
			tmpType = "Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			tmpType = "Push Group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			tmpType = "Pop Group";
			break;
		case GL_DEBUG_TYPE_OTHER:
			tmpType = "Other";
			break;
	};

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			tmpSeverity = "high";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			tmpSeverity = "medium";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			tmpSeverity = "low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			tmpSeverity = "notification";
			break;
	};
}

ModuleRender::ModuleRender() :
	context(nullptr),
	depthStencilRenderBuffer(0),
	bloomActivation(1),
	toneMappingMode(2),
	bloomIntensity(1.f),
	threshold(1.f)
{
}

ModuleRender::~ModuleRender()
{
	delete batchManager;
	delete gBuffer;
	delete shadows;
	delete ssao;
	delete lightPass;
	
	objectsInFrustrumDistances.clear();
	gameObjectsInFrustrum.clear();

	points.clear();
	spots.clear();
	spheres.clear();
}

bool ModuleRender::Init()
{
	ModuleWindow* window = App->GetModule<ModuleWindow>();
	LOG_VERBOSE("--------- Render Init ----------");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // desired version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // we want a double buffer
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	 // we want to have a depth buffer with 24 bits
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // we want to have a stencil buffer with 8 bits

	context = SDL_GL_CreateContext(window->GetWindow());

	backgroundColor = float4(0.f, 0.f, 0.f, 1.f);

	GLenum err = glewInit();
	// check for errors
	LOG_INFO("glew error {}", glewGetErrorString(err));
	// Should be 2.0
	LOG_INFO("Using Glew {}", glewGetString(GLEW_VERSION));

	LOG_INFO("Vendor: {}", glGetString(GL_VENDOR));
	LOG_INFO("Renderer: {}", glGetString(GL_RENDERER));
	LOG_INFO("OpenGL version supported {}", glGetString(GL_VERSION));
	LOG_INFO("GLSL: {}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&OurOpenGLErrorFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);

	glEnable(GL_DEPTH_TEST); // Enable depth test
	glEnable(GL_CULL_FACE); // Enable face culling
	glFrontFace(GL_CW);	 // Front faces will be clockwise
	glCullFace(GL_FRONT);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	batchManager = new BatchManager();
	gBuffer = new GBuffer();
	shadows = new Shadows();
	ssao = new SSAO();
	lightPass = new LightPass();

	glGenFramebuffers(1, &frameBuffer[0]);
	glGenTextures(1, &renderedTexture[0]);
#ifdef ENGINE
	glGenFramebuffers(1, &frameBuffer[1]);
	glGenTextures(1, &renderedTexture[1]);
#endif // ENGINE

	glGenFramebuffers(KAWASE_DUAL_SAMPLERS, dualKawaseDownFramebuffers);
	glGenTextures(KAWASE_DUAL_SAMPLERS, dualKawaseDownTextures);
	glGenFramebuffers(KAWASE_DUAL_SAMPLERS, dualKawaseUpFramebuffers);
	glGenTextures(KAWASE_DUAL_SAMPLERS, dualKawaseUpTextures);

	/*glGenFramebuffers(1, &bloomFramebuffer);
	glGenTextures(1, &bloomTexture);*/

	glGenRenderbuffers(1, &depthStencilRenderBuffer);

	shadows->InitBuffers();
	ssao->InitBuffers();

	std::pair<int, int> windowSize = window->GetWindowSize();
	UpdateBuffers(windowSize.first, windowSize.second);

	//Reserve space for Camera matrix
	glGenBuffers(1, &uboCamera);
	glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);

	const unsigned bindingCamera = 0;

	glBindBufferRange(GL_UNIFORM_BUFFER, bindingCamera, uboCamera, 0, sizeof(float4) * 8);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return true;
}

UpdateStatus ModuleRender::PreUpdate()
{
	int width, height;

	SDL_GetWindowSize(App->GetModule<ModuleWindow>()->GetWindow(), &width, &height);
	glViewport(0, 0, width, height);

	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
	
	gameObjectsInFrustrum.clear();
	objectsInFrustrumDistances.clear();

	points.clear();
	spots.clear();
	spheres.clear();
	tubes.clear();

	return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleRender::Update()
{
	if (App->GetModule<ModuleScene>()->IsLoading())
	{
		return UpdateStatus::UPDATE_CONTINUE;
	}

#ifdef DEBUG
	OPTICK_CATEGORY("UpdateRender", Optick::Category::Rendering);
#endif // DEBUG

	ModuleWindow* window = App->GetModule<ModuleWindow>();
	ModuleCamera* camera = App->GetModule<ModuleCamera>();
	ModuleDebugDraw* debug = App->GetModule<ModuleDebugDraw>();
	ModuleScene* scene = App->GetModule<ModuleScene>();
	ModulePlayer* modulePlayer = App->GetModule<ModulePlayer>();
	const ModuleProgram* modProgram = App->GetModule<ModuleProgram>();
	ModuleNavigation* navigation = App->GetModule<ModuleNavigation>();

	Scene* loadedScene = scene->GetLoadedScene();

	GameObject* player = modulePlayer->GetPlayer(); // we can make all of this variables a class variable to save time

	// Camera
	Camera* checkedCamera = GetFrustumCheckedCamera();
	Camera* engineCamera = camera->GetCamera();
	Frustum frustum = *engineCamera->GetFrustum();

#ifdef ENGINE
	if (App->GetPlayState() != Application::PlayState::STOPPED && player)
#else
	if (player)
#endif
	{
		AddToRenderList(player, checkedCamera);
	}

	GameObject* goSelected = App->GetModule<ModuleScene>()->GetSelectedGameObject();

	FillRenderList(App->GetModule<ModuleScene>()->GetLoadedScene()->GetRootQuadtree(), checkedCamera);
	
	std::vector<GameObject*> nonStaticsGOs = App->GetModule<ModuleScene>()->GetLoadedScene()->GetNonStaticObjects();

	for (GameObject* nonStaticObj : nonStaticsGOs)
	{
		AddToRenderList(nonStaticObj, checkedCamera);
	}
	
	if (goSelected)
	{
		AddToRenderList(goSelected, checkedCamera, true);
	}

	if (App->GetModule<ModuleDebugDraw>()->IsShowingBoundingBoxes())
	{
		DrawQuadtree(loadedScene->GetRootQuadtree());
	}

	int w, h;
	SDL_GetWindowSize(window->GetWindow(), &w, &h);

	glViewport(0, 0, w, h);

	// Bind camera and cubemap info to the shaders
	BindCubemapToProgram(modProgram->GetProgram(ProgramType::DEFAULT));
	BindCubemapToProgram(modProgram->GetProgram(ProgramType::SPECULAR));
	BindCubemapToProgram(modProgram->GetProgram(ProgramType::DEFERRED_LIGHT));
	BindCameraToProgram(modProgram->GetProgram(ProgramType::G_METALLIC), frustum);
	BindCameraToProgram(modProgram->GetProgram(ProgramType::G_SPECULAR), frustum);

	// -------- DEFERRED GEOMETRY -----------
	gBuffer->BindFrameBuffer();
	gBuffer->ClearFrameBuffer();

	glStencilMask(0x00); // disable writing to the stencil buffer 

	bool isRoot = goSelected != nullptr ? goSelected->GetParent() == nullptr : false;

	// Draw opaque objects
	batchManager->DrawOpaque(false);
	if (!isRoot && App->GetPlayState() == Application::PlayState::STOPPED)
	{
		// Draw selected opaque
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		batchManager->DrawOpaque(true);

		glDisable(GL_STENCIL_TEST);
	}

	// -------- SHADOW MAP --------
	if (shadows->UseShadows())
	{
		// ---- PARALLEL REDUCTION ----
		float2 minMax = shadows->ParallelReduction(gBuffer);
		shadows->RenderShadowMap(loadedScene->GetDirectionalLight(), minMax, engineCamera);

		if (shadows->UseVSM())
		{
			shadows->ShadowDepthVariance();
			shadows->GaussianBlur();
		}
	}
	
	Program* program;

	// SSAO Calculus and Blurring
	if (ssao->IsEnabled())
	{
		program = modProgram->GetProgram(ProgramType::SSAO);
		BindCameraToProgram(program, frustum);
		ssao->CalculateSSAO(program, w, h);

		ssao->BlurSSAO(w, h);
	}

	// -------- DEFERRED LIGHTING ---------------
	BindCameraToProgram(modProgram->GetProgram(ProgramType::DEFAULT), frustum);
	BindCameraToProgram(modProgram->GetProgram(ProgramType::SPECULAR), frustum);
	BindCameraToProgram(modProgram->GetProgram(ProgramType::DEFERRED_LIGHT), frustum);
	BindCameraToProgram(modProgram->GetProgram(ProgramType::LIGHT_CULLING), frustum);

	glPushDebugGroup
		(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(std::strlen("DEFERRED LIGHTING")), "DEFERRED LIGHTING");
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[0]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	program = modProgram->GetProgram(ProgramType::DEFERRED_LIGHT);
	program->Activate();

	gBuffer->BindTexture();

	// Binding Shadow map depth buffer
	if (shadows->UseShadows())
	{
		shadows->BindShadowMaps(program);

		ComponentDirLight* directLight = static_cast<ComponentDirLight*>(
			App->GetModule<ModuleScene>()->GetLoadedScene()->GetDirectionalLight()->GetComponentInternal<ComponentLight>());
		
		float2 shadowBias = directLight->GetShadowBias();

		program->BindUniformFloat("amount", directLight->GetBleedingAmount());

		if (!shadows->UseVSM())
		{
			program->BindUniformFloat("minBias", shadowBias[0]);
			program->BindUniformFloat("maxBias", shadowBias[1]);
		}
	}
	program->BindUniformInt("useShadows", static_cast<int>(shadows->UseShadows()));
	program->BindUniformInt("useVSM", static_cast<int>(shadows->UseVSM()));
	program->BindUniformInt("useSSAO", static_cast<int>(ssao->IsEnabled()));

	//Use to debug other Gbuffer/value default = 0 position = 1 normal = 2 diffuse = 3 specular = 4 and emissive = 5
	program->BindUniformInt("renderMode", modeRender);

	// SSAO texture
	if (ssao->IsEnabled())
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssao->GetSSAOTexture());
	}

	glDrawArrays(GL_TRIANGLES, 0, 3); // render Quad

	program->Deactivate();

	gBuffer->ReadFrameBuffer();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer[0]);
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer[0]);
	glPopDebugGroup();

	// ------- DEFERRED LIGHT PASS - Light culling ----------
	program = modProgram->GetProgram(ProgramType::LIGHT_CULLING);
	lightPass->RenderLights(program, gBuffer, modeRender, points, spots, spheres, tubes);
	// -----------------------------

	// -------- PRE-FORWARD ----------------------
	if (loadedScene->GetRoot()->HasComponent<ComponentSkybox>())
	{
		loadedScene->GetRoot()->GetComponentInternal<ComponentSkybox>()->
			Draw(engineCamera->GetViewMatrix(), engineCamera->GetProjectionMatrix());
	}

	debug->Draw(engineCamera->GetViewMatrix(), engineCamera->GetProjectionMatrix(), w, h);

	// -------- DEFERRED + FORWARD ---------------

	// Draw Transparent objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	batchManager->DrawTransparent(false);

	if (!isRoot && App->GetPlayState() == Application::PlayState::STOPPED)
	{
		// Draw selected transparent
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		batchManager->DrawTransparent(true);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //discard the ones that are previously captured
		glLineWidth(25);
		glPolygonMode(GL_BACK, GL_LINE);

		// Draw Highliht for selected objects
		if (goSelected)
		{
			DrawHighlight(goSelected);
		}

		glPolygonMode(GL_FRONT, GL_FILL);
		glLineWidth(1);
		glStencilMask(0x00); // disable writing to the stencil buffer 
		glDisable(GL_STENCIL_TEST);
	}

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_BACK, GL_FILL);

	// Draw Particles
	for (ComponentParticleSystem* particle : loadedScene->GetSceneParticleSystems())
	{
		particle->Render();
	}

	glEnable(GL_CULL_FACE); // Enable face culling
	glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT, GL_FILL);

	glDisable(GL_BLEND);

	//ComponentLine
	glDisable(GL_CULL_FACE);

	//additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (ComponentLine* lines : loadedScene->GetSceneComponentLines())
	{
		lines->Render();
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_BLEND);

	for (const GameObject* go : gameObjectsInFrustrum)
	{
		go->Render();
	}

	// ----- DRAW NAVMESH -----
	if (navigation->GetNavMesh() != nullptr && navigation->GetDrawNavMesh())
	{
		navigation->DrawGizmos();
	}

	// -------- POST EFFECTS ---------------------
	KawaseDualFiltering();

	// Color correction
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(std::strlen("Color correction")),
		"Color correction");
#ifdef ENGINE
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // default_frame_buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif // ENGINE

	Program* colorCorrectionProgram = modProgram->GetProgram(ProgramType::COLOR_CORRECTION);
	colorCorrectionProgram->Activate();
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dualKawaseUpTextures[KAWASE_DUAL_SAMPLERS - 1]);

	colorCorrectionProgram->BindUniformInt("tonneMappingMode", toneMappingMode);
	colorCorrectionProgram->BindUniformInt("bloomActivation", bloomActivation);
	colorCorrectionProgram->BindUniformFloat("bloomIntensity", bloomIntensity);

	glDrawArrays(GL_TRIANGLES, 0, 3); // render Quad
	colorCorrectionProgram->Deactivate();
	glPopDebugGroup();

	// ---- DRAW ALL COMPONENTS IN THE FRUSTRUM --

	for (const GameObject* go : gameObjectsInFrustrum)
	{
		go->Draw();
	}

#ifdef ENGINE
	ComponentCamera* frustumCheckedCamera = camera->GetFrustumCheckedCamera();
	if (frustumCheckedCamera && frustumCheckedCamera->GetCamera()->IsDrawFrustum())
	{
		frustumCheckedCamera->Draw();
	}
#endif // ENGINE

#ifndef ENGINE
	if (!App->IsDebuggingGame())
	{
		return UpdateStatus::UPDATE_CONTINUE;
	}
#endif //ENGINE

	return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleRender::PostUpdate()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_GL_SwapWindow(App->GetModule<ModuleWindow>()->GetWindow());

	return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleRender::CleanUp()
{
	LOG_VERBOSE("Destroying renderer");

	SDL_GL_DeleteContext(context);

	glDeleteBuffers(1, &uboCamera);

	glDeleteFramebuffers(1, &frameBuffer[0]);
	glDeleteTextures(1, &renderedTexture[0]);
#ifdef ENGINE
	glDeleteFramebuffers(1, &frameBuffer[1]);
	glDeleteTextures(1, &renderedTexture[1]);
#endif // ENGINE

	glDeleteFramebuffers(KAWASE_DUAL_SAMPLERS, dualKawaseDownFramebuffers);
	glDeleteTextures(KAWASE_DUAL_SAMPLERS, dualKawaseDownTextures);
	glDeleteFramebuffers(KAWASE_DUAL_SAMPLERS, dualKawaseUpFramebuffers);
	glDeleteTextures(KAWASE_DUAL_SAMPLERS, dualKawaseUpTextures);
	
	//glDeleteFramebuffers(1, &bloomFramebuffer);
	//glDeleteTextures(1, &bloomTexture);

	glDeleteRenderbuffers(1, &depthStencilRenderBuffer);

	return true;
}

void ModuleRender::WindowResized(unsigned width, unsigned height)
{
	App->GetModule<ModuleCamera>()->GetCamera()->SetAspectRatio(float(width) / height);
#ifdef ENGINE
	App->GetModule<ModuleEditor>()->Resized();
#endif // ENGINE
}

void ModuleRender::UpdateBuffers(unsigned width, unsigned height) //this is called twice
{
	gBuffer->InitGBuffer(width, height);
	shadows->UpdateBuffers(width, height);
	lightPass->SetScreenSize(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[0]);

	glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilRenderBuffer);

	glBindTexture(GL_TEXTURE_2D, renderedTexture[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	}

#ifdef ENGINE
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[1]);

	glBindTexture(GL_TEXTURE_2D, renderedTexture[1]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture[1], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer auxiliar is not complete!");
	}
#endif // ENGINE

	float auxWidht = static_cast<float>(width), auxHeight = static_cast<float>(height);

	for (unsigned int i = 0; i < KAWASE_DUAL_SAMPLERS; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, dualKawaseDownFramebuffers[i]);
		
		glBindTexture(GL_TEXTURE_2D, dualKawaseDownTextures[i]);
		
		auxWidht /= 2;
		auxHeight /= 2;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, static_cast<int>(auxWidht), static_cast<int>(auxHeight), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dualKawaseDownTextures[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer Dual Kawase down is not complete!");
		}
	}

	for (unsigned int i = 0; i < KAWASE_DUAL_SAMPLERS; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, dualKawaseUpFramebuffers[i]);

		glBindTexture(GL_TEXTURE_2D, dualKawaseUpTextures[i]);

		auxWidht *= 2;
		auxHeight *= 2;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, static_cast<int>(auxWidht), static_cast<int>(auxHeight), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dualKawaseUpTextures[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer Dual Kawase up is not complete!");
		}
	}

	/*glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffer);

	glBindTexture(GL_TEXTURE_2D, bloomTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer bloom is not complete!");
	}*/
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0); //Unbind the texture

	// SSAO
	ssao->UpdateBuffers(width, height);
	ssao->SetTextures(gBuffer->GetPositionTexture(), gBuffer->GetNormalTexture());
}

void ModuleRender::FillRenderList(const Quadtree* quadtree, Camera* camera)
{
	if (quadtree == nullptr)
	{
		return;
	}

	GameObject* player = App->GetModule<ModulePlayer>()->GetPlayer();

	/*if (player && quadtree->IsLeaf() && quadtree->InQuadrant(player))
	{
		quadtree->GetParent()->GetParent()->AddRigidBodiesToSimulation();
	}
	else
	{
		quadtree->RemoveRigidBodiesFromSimulation();
	}*/

		
	float3 cameraPos = camera->GetPosition();

	if (camera->IsInside(quadtree->GetBoundingBox()))
	{
		const std::set<GameObject*>& gameObjectsToRender = quadtree->GetGameObjects();
		if (quadtree->IsLeaf())
		{
			//quadtree->AddRigidBodiesToSimulation();
			for (const GameObject* gameObject : gameObjectsToRender)
			{
				ComponentTransform* transform = gameObject->GetComponentInternal<ComponentTransform>();
				// If an object doesn't have transform component it doesn't need to draw
				if (transform == nullptr)
				{
					return;
				}
				
				if (camera->IsInside(transform->GetEncapsuledAABB()))
				{
					if (gameObject->IsActive() && gameObject->IsEnabled())
					{
						float dist = Length(cameraPos - transform->GetGlobalPosition());

						gameObjectsInFrustrum.insert(gameObject);
						objectsInFrustrumDistances[gameObject] = dist;

						if (gameObject->HasComponent<ComponentLight>())
						{
							ComponentLight* light = gameObject->GetComponentInternal<ComponentLight>();

							if (!light->IsEnabled())
							{
								return;
							}

							switch (light->GetLightType())
							{
							case LightType::POINT:
								points.push_back(static_cast<ComponentPointLight*>(light));
								break;

							case LightType::SPOT:
								spots.push_back(static_cast<ComponentSpotLight*>(light));
								break;

								case LightType::AREA:
								{
									ComponentAreaLight* area = static_cast<ComponentAreaLight*>(light);
									switch (area->GetAreaType())
									{
									case AreaType::SPHERE:
										spheres.push_back(static_cast<ComponentAreaLight*>(light));
										break;

									case AreaType::TUBE:
										tubes.push_back(static_cast<ComponentAreaLight*>(light));
										break;
									}
									break;
								}
							}
						}
					}
				}

			}
		}
		else if (!gameObjectsToRender.empty()) //If the node is not a leaf but has GameObjects shared by all children
		{
			//quadtree->AddRigidBodiesToSimulation();
			for (const GameObject* gameObject : gameObjectsToRender)
			{
				ComponentTransform* transform = gameObject->GetComponentInternal<ComponentTransform>();
				// If an object doesn't have transform component it doesn't need to draw
				if (transform == nullptr)
				{
					return;
				}

				if (camera->IsInside(transform->GetEncapsuledAABB()))
				{
					if (gameObject->IsActive() && gameObject->IsEnabled())
					{
						float dist = Length(cameraPos - transform->GetGlobalPosition());

						gameObjectsInFrustrum.insert(gameObject);
						objectsInFrustrumDistances[gameObject] = dist;

						if (gameObject->HasComponent<ComponentLight>())
						{
							ComponentLight* light = gameObject->GetComponentInternal<ComponentLight>();

							if (!light->IsEnabled())
							{
								return;
							}

							switch (light->GetLightType())
							{
							case LightType::POINT:
								points.push_back(static_cast<ComponentPointLight*>(light));
								break;

							case LightType::SPOT:
								spots.push_back(static_cast<ComponentSpotLight*>(light));
								break;

								case LightType::AREA:
								{
									ComponentAreaLight* area = static_cast<ComponentAreaLight*>(light);
									switch (area->GetAreaType())
									{
									case AreaType::SPHERE:
										spheres.push_back(static_cast<ComponentAreaLight*>(light));
										break;

									case AreaType::TUBE:
										tubes.push_back(static_cast<ComponentAreaLight*>(light));
										break;
									}
									break;
								}
							}
						}
					}
				}

			}

			FillRenderList(quadtree->GetFrontRightNode(), camera); // And also call all the children to render
			FillRenderList(quadtree->GetFrontLeftNode(), camera);
			FillRenderList(quadtree->GetBackRightNode(), camera);
			FillRenderList(quadtree->GetBackLeftNode(), camera);
		}
		else
		{
			//quadtree->RemoveRigidBodiesFromSimulation();
			FillRenderList(quadtree->GetFrontRightNode(), camera);
			FillRenderList(quadtree->GetFrontLeftNode(), camera);
			FillRenderList(quadtree->GetBackRightNode(), camera);
			FillRenderList(quadtree->GetBackLeftNode(), camera);
		}
	}

	/*else
	{
		quadtree->RemoveRigidBodiesFromSimulation();
	}*/
}

void ModuleRender::AddToRenderList(const GameObject* gameObject, Camera* camera, bool recursive)
{
	float3 cameraPos = camera->GetPosition();

	if (gameObject->GetParent() == nullptr)
	{
		return;
	}

	ComponentTransform* transform = gameObject->GetComponentInternal<ComponentTransform>();
	// If an object doesn't have transform component it doesn't need to draw
	if (transform == nullptr)
	{
		return;
	}

	if (camera->IsInside(transform->GetEncapsuledAABB()))
	{
		ComponentMeshRenderer* mesh = gameObject->GetComponentInternal<ComponentMeshRenderer>();
		if (gameObject->IsActive() && (mesh == nullptr || mesh->IsEnabled()))
		{
			ComponentTransform* transform = gameObject->GetComponentInternal<ComponentTransform>();

			if (camera->IsInside(transform->GetEncapsuledAABB()))
			{
				if (gameObject->IsActive() && gameObject->IsEnabled())
				{
					float dist = Length(cameraPos - transform->GetGlobalPosition());

					gameObjectsInFrustrum.insert(gameObject);
					objectsInFrustrumDistances[gameObject] = dist;

					if (gameObject->HasComponent<ComponentLight>())
					{
						ComponentLight* light = gameObject->GetComponentInternal<ComponentLight>();

						if (!light->IsEnabled())
						{
							return;
						}

						switch (light->GetLightType())
						{
						case LightType::POINT:
							points.push_back(static_cast<ComponentPointLight*>(light));
							break;

						case LightType::SPOT:
							spots.push_back(static_cast<ComponentSpotLight*>(light));
							break;

							case LightType::AREA:
							{
								ComponentAreaLight* area = static_cast<ComponentAreaLight*>(light);
								switch (area->GetAreaType())
								{
								case AreaType::SPHERE:
									spheres.push_back(static_cast<ComponentAreaLight*>(light));
									break;

								case AreaType::TUBE:
									tubes.push_back(static_cast<ComponentAreaLight*>(light));
									break;
								}
								break;
							}
						}
					}
				}
			}
		}
	}

	if (recursive && !gameObject->GetChildren().empty())
	{
		for (GameObject* children : gameObject->GetChildren())
		{
			AddToRenderList(children, camera, recursive);
		}
	}
}

void ModuleRender::DrawQuadtree(const Quadtree* quadtree)
{
#ifdef ENGINE
	if (quadtree->IsLeaf())
	{
		App->GetModule<ModuleDebugDraw>()->DrawBoundingBox(quadtree->GetBoundingBox());
	}
	else
	{
		DrawQuadtree(quadtree->GetBackLeftNode());
		DrawQuadtree(quadtree->GetBackRightNode());
		DrawQuadtree(quadtree->GetFrontLeftNode());
		DrawQuadtree(quadtree->GetFrontRightNode());
	}
#endif // ENGINE
}

void ModuleRender::DrawMeshesByFilter(std::vector<GameObject*>& objects, ProgramType type, bool normalBehaviour)
{
	ModuleProgram* modProgram = App->GetModule<ModuleProgram>();
	Program* program;
	int filter;
	switch (type)
	{
	case ProgramType::DEFAULT:
		program = modProgram->GetProgram(ProgramType::DEFAULT);
		filter = batchManager->HAS_METALLIC;
		if (normalBehaviour)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			filter |= batchManager->HAS_TRANSPARENCY;
		}
		else
		{
			filter |= batchManager->HAS_OPAQUE;
		}
		program->Activate();
		batchManager->DrawMeshesByFilters(objects, filter);
		program->Deactivate();
		glDisable(GL_BLEND);
		break;
	
	case ProgramType::SPECULAR:
		program = modProgram->GetProgram(ProgramType::SPECULAR);
		filter = batchManager->HAS_SPECULAR;
		if (normalBehaviour)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			filter |= batchManager->HAS_TRANSPARENCY;
		}
		else
		{
			filter |= batchManager->HAS_OPAQUE;
		}
		program->Activate();
		batchManager->DrawMeshesByFilters(objects, filter);
		program->Deactivate();
		glDisable(GL_BLEND);
		break;
	
	case ProgramType::G_METALLIC:
		program = modProgram->GetProgram(ProgramType::DEFAULT);
		filter = batchManager->HAS_METALLIC | batchManager->HAS_OPAQUE;
		program->Activate();
		batchManager->DrawMeshesByFilters(objects, filter);
		program->Deactivate();
		break;
	
	case ProgramType::G_SPECULAR:
		program = modProgram->GetProgram(ProgramType::SPECULAR);
		filter = batchManager->HAS_SPECULAR | batchManager->HAS_OPAQUE;
		program->Activate();
		batchManager->DrawMeshesByFilters(objects, filter);
		program->Deactivate();
		break;
	
	default:
		break;
	}
}

void ModuleRender::SortOpaques(std::vector<GameObject*>& sceneGameObjects, const float3& pos)
{
	std::sort(sceneGameObjects.begin(), sceneGameObjects.end(),
		[pos](GameObject*& a, GameObject*& b) -> bool
		{
			float aDist = a->GetComponentInternal<ComponentTransform>()->GetGlobalPosition().DistanceSq(pos);
			float bDist = b->GetComponentInternal<ComponentTransform>()->GetGlobalPosition().DistanceSq(pos);

			return aDist < bDist;
		});
}

void ModuleRender::SortTransparents(std::vector<GameObject*>& sceneGameObjects, const float3& pos)
{
	std::sort(sceneGameObjects.begin(), sceneGameObjects.end(),
		[pos](GameObject*& a, GameObject*& b) -> bool
		{
			float aDist = a->GetComponentInternal<ComponentTransform>()->GetGlobalPosition().DistanceSq(pos);
			float bDist = b->GetComponentInternal<ComponentTransform>()->GetGlobalPosition().DistanceSq(pos);

			return aDist > bDist;
		});
}

void ModuleRender::DrawHighlight(GameObject* gameObject)
{
	std::queue<GameObject*> gameObjectQueue;
	gameObjectQueue.push(gameObject);

	while (!gameObjectQueue.empty())
	{
		const GameObject* currentGo = gameObjectQueue.front();
		gameObjectQueue.pop();
		for (GameObject* child : currentGo->GetChildren())
		{
			if (child->IsEnabled())
			{
				gameObjectQueue.push(child);
			}
		}
		std::vector<ComponentMeshRenderer*> meshes = currentGo->GetComponents<ComponentMeshRenderer>();
		
		if (gameObjectsInFrustrum.find(currentGo) != gameObjectsInFrustrum.end())
		{
			for (const ComponentMeshRenderer* mesh : meshes)
			{
				mesh->DrawHighlight();
			}
		}
	}
}

void ModuleRender::BindCameraToProgram(Program* program, Frustum& frustum)
{
	program->Activate();

	const float4x4& view = frustum.ViewMatrix();
	const float4x4& proj = frustum.ProjectionMatrix();
	float3 viewPos = frustum.Pos();

	glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float4) * 4, &proj);
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(float4) * 4, &view);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	program->BindUniformFloat3("viewPos", viewPos);

	program->Deactivate();
}

void ModuleRender::BindCubemapToProgram(Program* program)
{
	Cubemap* cubemap = App->GetModule<ModuleScene>()->GetLoadedScene()->GetCubemap();

	if (cubemap == nullptr)
	{
		return;
	}

	program->Activate();

	ComponentSkybox* sky = App->GetModule<ModuleScene>()->GetLoadedScene()
		->GetRoot()->GetComponentInternal<ComponentSkybox>();

	if (sky && sky->GetUseCubeMap())
	{
		Cubemap* skyCubemap = sky->GetCubemap();
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap->GetIrradiance());
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap->GetPrefiltered());
	}
	else
	{
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetIrradiance());
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetPrefiltered());
		
	}
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, cubemap->GetEnvironmentBRDF());

	program->BindUniformInt("numLevels_IBL", cubemap->GetNumMiMaps());
	program->BindUniformFloat("cubemap_intensity", cubemap->GetIntensity());

	program->Deactivate();
}

void ModuleRender::KawaseDualFiltering()
{
	// Blur bloom with kawase
	ModuleProgram* moduleProgram = App->GetModule<ModuleProgram>();
	
	//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(std::strlen("Bloom")),
	//	"Bloom");
	//glBindFramebuffer(GL_FRAMEBUFFER, bloomFramebuffer);
	//Program* bloom = moduleProgram->GetProgram(ProgramType::BLOOM);
	//bloom->Activate();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, renderedTexture[0]);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, gBuffer->GetEmissiveTexture());

	//bloom->BindUniformFloat("threshold", threshold);
	//glDrawArrays(GL_TRIANGLES, 0, 3); // render Quad
	//bloom->Deactivate();
	//glPopDebugGroup();

	ModuleWindow* moduleWindow = App->GetModule<ModuleWindow>();

	std::pair<int, int> windowSize = moduleWindow->GetWindowSize();
	
	float auxWidht = static_cast<float>(windowSize.first), auxHeight = static_cast<float>(windowSize.second);
	bool firstIteration = true;
	
	Program* kawaseDownProgram = moduleProgram->GetProgram(ProgramType::KAWASE_DOWN);
	kawaseDownProgram->Activate();
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(std::strlen("Kawase dual down")),
		"Kawase dual down");
	for (auto i = 0; i < KAWASE_DUAL_SAMPLERS; ++i)
	{
		auxWidht /= 2;
		auxHeight /= 2;
		glViewport(0, 0, static_cast<int>(auxWidht), static_cast<int>(auxHeight));

		glBindFramebuffer(GL_FRAMEBUFFER, dualKawaseDownFramebuffers[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? gBuffer->GetEmissiveTexture() : dualKawaseDownTextures[i - 1]);

		glDrawArrays(GL_TRIANGLES, 0, 3); // render Quad

		firstIteration = false;
	}
	kawaseDownProgram->Deactivate();
	glPopDebugGroup();

	firstIteration = true;
	Program* kawaseUpProgram = moduleProgram->GetProgram(ProgramType::KAWASE_UP);
	kawaseUpProgram->Activate();
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(std::strlen("Kawase dual up")),
		"Kawase dual up");
	for (auto i = 0; i < KAWASE_DUAL_SAMPLERS; ++i)
	{
		auxWidht *= 2;
		auxHeight *= 2;
		glViewport(0, 0, static_cast<int>(auxWidht), static_cast<int>(auxHeight));

		glBindFramebuffer(GL_FRAMEBUFFER, dualKawaseUpFramebuffers[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstIteration ? dualKawaseDownTextures[KAWASE_DUAL_SAMPLERS - 1]
			: dualKawaseUpTextures[i - 1]);

		glDrawArrays(GL_TRIANGLES, 0, 3); // render Quad

		firstIteration = false;
	}
	kawaseUpProgram->Deactivate();
	glPopDebugGroup();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, windowSize.first, windowSize.second);
}

Camera* ModuleRender::GetFrustumCheckedCamera() const
{
	ModuleCamera* moduleCamera = App->GetModule<ModuleCamera>();
	ComponentCamera* frustumCheckedCamera = moduleCamera->GetFrustumCheckedCamera();
	if (!frustumCheckedCamera)
	{
		return moduleCamera->GetCamera();
	}
	else
	{
		return (Camera*) frustumCheckedCamera->GetCamera();
	}
}

bool ModuleRender::CheckIfTransparent(const GameObject* gameObject)
{
	const ComponentMeshRenderer* material = gameObject->GetComponentInternal<ComponentMeshRenderer>();
	if (material != nullptr && material->GetMaterial() != nullptr)
	{
		if (!material->GetMaterial()->IsTransparent())
			return false;
		else
			return true;
	}

	return false;
}
