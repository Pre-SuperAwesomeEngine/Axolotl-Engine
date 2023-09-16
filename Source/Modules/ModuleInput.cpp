#include "StdAfx.h"

#include "Modules/ModuleInput.h"

#include "Application.h"
#include "AxoLog.h"
#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleUI.h"
#include "Scene/Scene.h"
#include "Windows/WindowMainMenu.h"
#include "imgui_impl_sdl.h"

#ifdef DEBUG
	#include "optick.h"
#endif // DEBUG

namespace
{
const std::map<SDL_GameControllerButton, std::variant<SDL_Scancode, Uint8>> gamepadMapping = {
	{ SDL_CONTROLLER_BUTTON_A, SDL_SCANCODE_SPACE },
	{ SDL_CONTROLLER_BUTTON_B, SDL_SCANCODE_E },
	{ SDL_CONTROLLER_BUTTON_X, static_cast<Uint8>(SDL_BUTTON_LEFT) },
	{ SDL_CONTROLLER_BUTTON_Y, static_cast<Uint8>(SDL_BUTTON_RIGHT) },
	{ SDL_CONTROLLER_BUTTON_BACK, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_GUIDE, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_START, SDL_SCANCODE_ESCAPE },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_SCANCODE_X },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_SCANCODE_LSHIFT },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_SCANCODE_Z },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_SCANCODE_TAB },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_MISC1, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_PADDLE1, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_PADDLE2, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_PADDLE3, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_PADDLE4, SDL_SCANCODE_UNKNOWN },
	{ SDL_CONTROLLER_BUTTON_TOUCHPAD, SDL_SCANCODE_UNKNOWN },
};
} // namespace

ModuleInput::ModuleInput() :
	mouseWheel(float2::zero),
	mouseMotion(float2::zero),
	mousePosX(0),
	mousePosY(0),
	direction{ JoystickHorizontalDirection::NONE, JoystickVerticalDirection::NONE }
{
}

ModuleInput::~ModuleInput()
{
}

bool ModuleInput::Init()
{
	LOG_VERBOSE("Init SDL input event system");
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG_ERROR("SDL_EVENTS could not initialize! SDL_Error: {}\n", SDL_GetError());
		return false;
	}

#ifdef ENGINE
	freeLookSurface = std::unique_ptr<SDL_Surface, SDLSurfaceDestroyer>(SDL_LoadBMP(BMP_FREELOOKSURFACE));
	orbitSurface = std::unique_ptr<SDL_Surface, SDLSurfaceDestroyer>(SDL_LoadBMP(BMP_ORBITSURFACE));
	moveSurface = std::unique_ptr<SDL_Surface, SDLSurfaceDestroyer>(SDL_LoadBMP(BMP_MOVESURFACE));
	zoomSurface = std::unique_ptr<SDL_Surface, SDLSurfaceDestroyer>(SDL_LoadBMP(BMP_ZOOMSURFACE));
	freeLookCursor =
		std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(SDL_CreateColorCursor(freeLookSurface.get(), 0, 0));
	orbitCursor = std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(SDL_CreateColorCursor(orbitSurface.get(), 0, 0));
	moveCursor = std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(SDL_CreateColorCursor(moveSurface.get(), 0, 0));
	zoomCursor = std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(SDL_CreateColorCursor(zoomSurface.get(), 0, 0));

	std::unique_ptr<SDL_Cursor, SDLCursorDestroyer> defaultCursor =
		std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(SDL_GetCursor());
	this->defaultCursor = std::unique_ptr<SDL_Cursor, SDLCursorDestroyer>(defaultCursor.get());
#else  // ENGINE
	SetShowCursor(false);
#endif // GAMEMODE

	return true;
}

UpdateStatus ModuleInput::PreUpdate()
{
	mouseMotion = float2::zero;
	mouseWheelScrolled = false;

for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		if (keysState[i] == KeyState::DOWN)
		{
			keysState[i] = KeyState::REPEAT;
		}

		if (keysState[i] == KeyState::UP)
		{
			keysState[i] = KeyState::IDLE;
		}
	}

	for (int i = 0; i < NUM_MOUSEBUTTONS; ++i)
	{
		if (mouseButtonState[i] == KeyState::DOWN)
		{
			mouseButtonState[i] = KeyState::REPEAT;
		}

		if (mouseButtonState[i] == KeyState::UP)
		{
			mouseButtonState[i] = KeyState::IDLE;
		}
	}

	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
	{
		if (gamepadState[i] == KeyState::DOWN)
		{
			gamepadState[i] = KeyState::REPEAT;
		}

		if (gamepadState[i] == KeyState::UP)
		{
			gamepadState[i] = KeyState::IDLE;
		}
	}
	SDL_PumpEvents();

	SDL_GameController* controller = FindController();

	SDL_Event sdlEvent;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
#ifdef ENGINE
		ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
#else // ENGINE

		if (App->IsDebuggingGame())
		{
			ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
		}
#endif

		switch (sdlEvent.type)
		{
			case SDL_QUIT:
				return UpdateStatus::UPDATE_STOP;

			case SDL_WINDOWEVENT:
				if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED ||
					sdlEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					ModuleRender* render = App->GetModule<ModuleRender>();
					render->WindowResized(sdlEvent.window.data1, sdlEvent.window.data2);
					render->UpdateBuffers(sdlEvent.window.data1, sdlEvent.window.data2);
					App->GetModule<ModuleUI>()->RecalculateCanvasSizeAndScreenFactor();
					App->GetModule<ModuleCamera>()->RecalculateOrthoProjectionMatrix();
				}
				if (sdlEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					inFocus = false;
				}
				if (sdlEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					inFocus = true;
				}
				break;

			case SDL_KEYDOWN:
				if (sdlEvent.key.repeat == 0)
				{
					keysState[sdlEvent.key.keysym.scancode] = KeyState::DOWN;
				}
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_KEYUP:
				if (sdlEvent.key.repeat == 0)
				{
					keysState[sdlEvent.key.keysym.scancode] = KeyState::UP;
				}
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouseButtonState[sdlEvent.button.button] = KeyState::DOWN;
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_MOUSEBUTTONUP:
				mouseButtonState[sdlEvent.button.button] = KeyState::UP;
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_MOUSEMOTION:
				mousePosX = sdlEvent.motion.x;
				mousePosY = sdlEvent.motion.y;
				mouseMotion = float2((float) sdlEvent.motion.xrel, (float) sdlEvent.motion.yrel);
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_MOUSEWHEEL:
				mouseWheel = float2((float) sdlEvent.wheel.x, (float) sdlEvent.wheel.y);
				mouseWheelScrolled = true;
				inputMethod = InputMethod::KEYBOARD;
				break;

			case SDL_CONTROLLERDEVICEADDED:
				if (!controller)
				{
					controller = SDL_GameControllerOpen(sdlEvent.cdevice.which);
					inputMethod = InputMethod::GAMEPAD;
				}
				break;

			case SDL_CONTROLLERDEVICEREMOVED:
				if (controller && sdlEvent.cdevice.which == GetControllerInstanceID(controller))
				{
					SDL_GameControllerClose(controller);
					controller = FindController();
				}
				if (!controller)
				{
					inputMethod = InputMethod::KEYBOARD;
				}
				break;

			case SDL_CONTROLLERBUTTONDOWN:
				if (controller && sdlEvent.cdevice.which == GetControllerInstanceID(controller))
				{
					gamepadState[sdlEvent.cbutton.button] = KeyState::DOWN;
				}
				inputMethod = InputMethod::GAMEPAD;
				break;

			case SDL_CONTROLLERBUTTONUP:
				if (controller && sdlEvent.cdevice.which == GetControllerInstanceID(controller))
				{
					gamepadState[sdlEvent.cbutton.button] = KeyState::UP;
				}
				inputMethod = InputMethod::GAMEPAD;
				break;

			case SDL_CONTROLLERAXISMOTION:
				if (controller)
				{
					axis = static_cast<SDL_GameControllerAxis>(sdlEvent.caxis.axis);
					axisValue = sdlEvent.caxis.value;
					if (axis == SDL_CONTROLLER_AXIS_LEFTX)
					{
						if (axisValue > 3200)
						{
							direction.horizontalMovement = JoystickHorizontalDirection::RIGHT;
							inputMethod = InputMethod::GAMEPAD;
						}
						else if (axisValue < -3200)
						{
							direction.horizontalMovement = JoystickHorizontalDirection::LEFT;
							inputMethod = InputMethod::GAMEPAD;
						}
						else
						{
							direction.horizontalMovement = JoystickHorizontalDirection::NONE;
						}
					}

					if (axis == SDL_CONTROLLER_AXIS_LEFTY)
					{
						if (axisValue < -3200)
						{
							direction.verticalMovement = JoystickVerticalDirection::FORWARD;
							inputMethod = InputMethod::GAMEPAD;
						}
						else if (axisValue > 3200)
						{
							direction.verticalMovement = JoystickVerticalDirection::BACK;
							inputMethod = InputMethod::GAMEPAD;
						}
						else
						{
							direction.verticalMovement = JoystickVerticalDirection::NONE;
						}
					}
				}
				break;

			case SDL_DROPFILE:
				char* droppedFilePathAsChar = sdlEvent.drop.file;

				std::string droppedFilePathString(droppedFilePathAsChar);
				std::replace(droppedFilePathString.begin(), droppedFilePathString.end(), '\\', '/');
				App->GetModule<ModuleScene>()->GetLoadedScene()->ConvertModelIntoGameObject(droppedFilePathString);
				SDL_free(droppedFilePathAsChar); // Free dropped_filedir memory
				break;
		}
	}

	MapInput();

	return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleInput::Update()
{
#ifdef DEBUG
	OPTICK_CATEGORY("UpdateInput", Optick::Category::Input);
#endif // DEBUG

	if (keysState[SDL_SCANCODE_LALT] == KeyState::REPEAT && keysState[SDL_SCANCODE_J] == KeyState::DOWN)
	{
		App->SwitchDebuggingGame();
#ifndef ENGINE
		if (!App->IsDebuggingGame())
		{
			SetShowCursor(false);
		}
#endif // ENGINE
	}

#ifdef ENGINE
	if ((keysState[SDL_SCANCODE_LCTRL] == KeyState::REPEAT || keysState[SDL_SCANCODE_LCTRL] == KeyState::DOWN) &&
		keysState[SDL_SCANCODE_Q] == KeyState::DOWN)
	{
		if (App->IsOnPlayMode())
		{
			App->OnStop();
		}
	}

	if ((keysState[SDL_SCANCODE_LCTRL] == KeyState::REPEAT || keysState[SDL_SCANCODE_LCTRL] == KeyState::DOWN) &&
		keysState[SDL_SCANCODE_A] == KeyState::DOWN)
	{
		if (App->IsOnPlayMode())
		{
			SDL_ShowCursor(SDL_QUERY) ? SetShowCursor(false) : SetShowCursor(true);
		}
	}

	if (keysState[SDL_SCANCODE_LCTRL] == KeyState::REPEAT && keysState[SDL_SCANCODE_S] == KeyState::DOWN &&
		SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleEditor>()->GetMainMenu()->ShortcutSave();
	}

	if (keysState[SDL_SCANCODE_F1] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->SwitchBloomActivation();
	}

	if (keysState[SDL_SCANCODE_F2] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ChangeToneMapping();
	}

	if (keysState[SDL_SCANCODE_F3] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ToggleShadows();
	}

	if (keysState[SDL_SCANCODE_F4] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ToggleVSM();
	}

	if (keysState[SDL_SCANCODE_F5] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ChangeRenderMode();
	}

	if (keysState[SDL_SCANCODE_F6] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ToggleSSAO();
	}

	if (keysState[SDL_SCANCODE_F7] == KeyState::DOWN && SDL_ShowCursor(SDL_QUERY))
	{
		App->GetModule<ModuleRender>()->ToggleCSMDebug();
	}

#endif

	return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleInput::CleanUp()
{
	LOG_VERBOSE("Quitting SDL input event subsystem");

	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return true;
}

void ModuleInput::MapInput()
{
	if (App->IsOnPlayMode())
	{
		for (const auto& [gamepadButton, keyboardButton] : gamepadMapping)
		{
			KeyState gamepadButtonState = GetGamepadButton(gamepadButton);

			if (gamepadButtonState == KeyState::IDLE)
			{
				continue;
			}

			if (std::holds_alternative<SDL_Scancode>(keyboardButton))
			{
				keysState[std::get<SDL_Scancode>(keyboardButton)] = gamepadButtonState;
			}
			else if (std::holds_alternative<Uint8>(keyboardButton))
			{
				Uint8 mouseButton = std::get<Uint8>(keyboardButton);
				mouseButtonState[mouseButton] = gamepadButtonState;
			}
		}
	}
}
