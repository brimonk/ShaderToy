/*
 * Brian Chrzanowski
 * 2020-08-31 23:34:47
 *
 * Brian's Frag Shader Fun - Without the X11
 */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#define COMMON_IMPLEMENTATION
#include "common.h"

#define SCREEN_WIDTH  (1280)
#define SCREEN_HEIGHT (960)

struct bundlemeta_t {
	// shader id
	s32 prog;

	// vertex array and buffer objects
	u32 vao, vbo;

	// shader location values
	s32 lRes;
	s32 lTime;
	s32 lTick;
	s32 lMouse;

	// paths to shader parts
	char *fsrc_path;

	// misc program state
	s32 reload;
	s32 run;
	s32 draw;
};

struct bundlevals_t {
	s32 uWin_w, uWin_h; // window dimensions
	f32 uTime; // current "time" (from start)
	s32 uTick; // current "tick"
	s32 uMouse_x, uMouse_y;
};

// STARTUP / SHUTDOWN FUNCTIONS
/* Init : Initializes the Game State */
s32 Init();
/* Close : Closes the Game */
s32 Close();

/* Update : updates the state machine */
s32 Update(struct bundlemeta_t *meta, struct bundlevals_t *vals);

/* Render : Rendering Function */
s32 Render(struct bundlemeta_t *meta, struct bundlevals_t *vals);

// GL Helper Functions
/* Shader_Setup: sets up the shader, and binds to the shader's uniforms */
s32 Shader_Setup(struct bundlemeta_t *meta, struct bundlevals_t *vals);
/* Shader_Defaults : set meta & vals defaults */
s32 Shader_Defaults(struct bundlemeta_t *meta, struct bundlevals_t *vals);
/* Shader_Compile : compile shader from vertex and fragment sources */
s32 Shader_Compile(char *vsrc, char *fsrc);
/* Shader_ProgramLog : Prints the Log for the Shader */
s32 Shader_ProgramLog(GLint program);
/* Shader_ShaderLog : prints the shader log */
s32 Shader_ShaderLog(GLuint shader);

// INPUT FUNCTIONS
/* Input_HandleKeys : Handles the Keyboard */
s32 Input_HandleKeys(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals);
/* Input_HandleMouse : Handles the Mouse */
s32 Input_HandleMouse(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals);
/* Input_HandleWindow : Handles Window Events */
s32 Input_HandleWindow(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals);

#define USAGE ("USAGE: %s <frag>\n")

SDL_Window *gWindow = NULL;
SDL_GLContext gContext;

int main(int argc, char **argv)
{
	SDL_Event event;
	struct bundlemeta_t meta;
	struct bundlevals_t vals;

	memset(&meta, 0, sizeof meta);
	memset(&vals, 0, sizeof vals);

	if (argc < 2) {
		ERR(USAGE, argv[0]);
		return 1;
	}

	meta.fsrc_path = argv[1];

	if (Init()) {
		ERR("Couldn't initialize the game. Quitting...\n");
		return 1;
	}

	for (meta.reload = meta.run = 1; meta.run; ) {

		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					meta.run = 0;
					break;

				case SDL_MOUSEMOTION:
					Input_HandleMouse(&event, &meta, &vals);
					break;

				case SDL_WINDOWEVENT:
					Input_HandleWindow(&event, &meta, &vals);
					break;

				case SDL_KEYDOWN:
					Input_HandleKeys(&event, &meta, &vals);
					break;

				default:
					break;
			}
		}

		if (meta.reload) {
			Shader_Setup(&meta, &vals);
			meta.reload = 0;
		}

		Update(&meta, &vals);

		Render(&meta, &vals);
	}

	Close();

	return 0;
}

/* Update : updates the state machine */
s32 Update(struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	assert(meta);
	assert(vals);

	glUseProgram(meta->prog);

	// update the state
	vals->uTick++;
	vals->uTime = SDL_GetTicks() / 1000.0f;

	// set the viewport
	glViewport(0, 0, vals->uWin_w, vals->uWin_h);

	// bind the state
	if (0 <= meta->lTime) {
		glUniform1f(meta->lTime, vals->uTime);
	}

	if (0 <= meta->lTick) {
		glUniform1i(meta->lTick, vals->uTick);
	}

	if (0 <= meta->lRes) {
		glUniform2i(meta->lRes, vals->uWin_w, vals->uWin_h);
	}

	if (0 <= meta->lMouse) {
		glUniform2i(meta->lMouse, vals->uMouse_x, vals->uMouse_y);
	}

	glUseProgram(0);

	return 0;
}

/* Render : Rendering Function */
s32 Render(struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(meta->prog);

	glBindBuffer(GL_ARRAY_BUFFER, meta->vbo);
	glBindVertexArray(meta->vao);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);

	SDL_GL_SwapWindow(gWindow);

	return 0;
}

/* Shader_Setup: sets up the shader, and binds to the shader's uniforms */
s32 Shader_Setup(struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	char *vsrc, *fsrc;
	s32 w, h;

	// NOTE (brian)
	//   maintains the shader programs, both vertex and fragmentation

	assert(meta);
	assert(vals);

	glDeleteBuffers(1, &meta->vbo);
	glDeleteBuffers(1, &meta->vao);

	glDeleteProgram(meta->prog);

	// TODO (brian) we probably just want an actual function to reset, not just memsetting
	// everything to zero. it'd make this easier, maybe
	vsrc = meta->fsrc_path;
	fsrc = sys_readfile(meta->fsrc_path);
	w = vals->uWin_w ? vals->uWin_w : SCREEN_WIDTH;
	h = vals->uWin_h ? vals->uWin_h : SCREEN_HEIGHT;

	memset(meta, 0, sizeof(*meta));
	memset(vals, 0, sizeof(*vals));

	Shader_Defaults(meta, vals);

	meta->fsrc_path = vsrc;
	meta->run = 1;

	vsrc = "#version 330\nlayout (location = 0) in vec2 vPos; void main() { gl_Position = vec4(vPos, 0.0, 1.0); }";

	// make sure we set defaults / constants
	vals->uWin_w = w;
	vals->uWin_h = h;

	meta->prog = Shader_Compile(vsrc, fsrc);

	free(fsrc);

	f32 verts[] = {
		-1.0, -1.0,
		-1.0,  1.0,
		 1.0, -1.0,
		-1.0,  1.0,
		 1.0,  1.0,
		 1.0, -1.0
	};

	// generate vertex buffer objects
	glGenBuffers(1, &meta->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, meta->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

	// generate vertex buffer arrays
	glGenVertexArrays(1, &meta->vao);
	glBindVertexArray(meta->vao);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	glUseProgram(meta->prog);

	// fetch the locations of variables
	meta->lTime  = glGetUniformLocation(meta->prog, "uTime");
	meta->lTick  = glGetUniformLocation(meta->prog, "uTick");
	meta->lRes   = glGetUniformLocation(meta->prog, "uRes");
	meta->lMouse = glGetUniformLocation(meta->prog, "uMouse");

	glUseProgram(0);

	return 0;
}

/* Shader_Defaults : set shader defaults */
s32 Shader_Defaults(struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	meta->run = 1;
	meta->draw = 1;

	SDL_GetWindowSize(gWindow, &vals->uWin_w, &vals->uWin_h);

	return 0;
}

/* Input_HandleKeys : Handles the Keyboard */
s32 Input_HandleKeys(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	s32 rc;

	assert(event);
	assert(meta);
	assert(vals);
	rc = 0;

	// NOTE (brian)
	//   https://wiki.libsdl.org/SDL_Keycode

	if (event->type != SDL_KEYDOWN) {
		return -1;
	}

	// just the first key
	switch (event->key.keysym.scancode) {
		case SDL_SCANCODE_Q:
		{
			meta->run = 0;
			break;
		}

		case SDL_SCANCODE_R:
		{
			meta->reload = 1;
			break;
		}

		default:
		{
			break;
		}
	}

	return rc;
}

/* Input_HandleMouse : Handles the Mouse */
s32 Input_HandleMouse(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	assert(meta);
	assert(vals);

	if (event->type != SDL_MOUSEMOTION) {
		return -1;
	}

	SDL_GetMouseState(&vals->uMouse_x, &vals->uMouse_y);

	return 0;
}

/* Input_HandleWindow : Handles Window Events */
s32 Input_HandleWindow(SDL_Event *event, struct bundlemeta_t *meta, struct bundlevals_t *vals)
{
	assert(event);
	assert(meta);
	assert(vals);

	if (event->type != SDL_WINDOWEVENT) {
		return -1;
	}

	switch (event->window.event) {
	case SDL_WINDOWEVENT_SHOWN:
		meta->draw = 1;
		break;
	case SDL_WINDOWEVENT_HIDDEN:
		meta->draw = 1;
		break;
	case SDL_WINDOWEVENT_EXPOSED:
		meta->draw = 1;
		break;
	case SDL_WINDOWEVENT_MOVED:
		break;
	case SDL_WINDOWEVENT_RESIZED:
		vals->uWin_w = event->window.data1;
		vals->uWin_h = event->window.data2;
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		vals->uWin_w = event->window.data1;
		vals->uWin_h = event->window.data2;
		break;
	case SDL_WINDOWEVENT_MINIMIZED:
		break;
	case SDL_WINDOWEVENT_MAXIMIZED:
		break;
	case SDL_WINDOWEVENT_RESTORED:
		meta->draw = 1;
		break;
	case SDL_WINDOWEVENT_ENTER:
		break;
	case SDL_WINDOWEVENT_LEAVE:
		break;
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		meta->draw = 1;
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		meta->draw = 0;
		break;
	default:
		SDL_Log("Window %d got unknown event %d", event->window.windowID, event->window.event);
		break;
	}

	return 0;
}

/* Init : Initializes the Game State */
s32 Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		ERR("Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	gWindow = SDL_CreateWindow("ShaderToy", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (gWindow == NULL) { // error
		ERR("Couldn't Create Window: %s\n", SDL_GetError());
		return 1;
	}

	gContext = SDL_GL_CreateContext(gWindow);
	if (gContext == NULL) {
		ERR("Couldn't Create OpenGL Context: %s\n", SDL_GetError());
		return 1;
	}

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		ERR("Couldn't Create OpenGL Context: %s\n", glewGetErrorString(glewError));
		return 1;
	}

	if (SDL_GL_SetSwapInterval(1) < 0) {
		WRN("Couldn't Set VSync: %s\n", SDL_GetError());
	}

	glClearColor(0.f, 0.f, 0.f, 1.f);

	return 0;
}

/* Close : closes the game */
s32 Close()
{
	SDL_DestroyWindow(gWindow);
	SDL_Quit();

	return 0;
}

/* Shader_Compile : compile shader from vertex and fragment sources */
s32 Shader_Compile(char *vsrc, char *fsrc)
{
	GLint pShader;
	GLuint vShader, fShader;
	GLint rc;

	pShader = glCreateProgram();
	   
	// vertex shader stuff
	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, (const GLchar *const *)&vsrc, NULL);
	glCompileShader(vShader);

	rc = GL_FALSE;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &rc);
	if (rc != GL_TRUE) {
		ERR("Couldn't Compile Vert Shader %d!\n", vShader);
		Shader_ShaderLog(vShader);
		return -1;
	}

	glAttachShader(pShader, vShader);

	// fragment shader stuff
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, (const GLchar *const *)&fsrc, NULL);
	glCompileShader(fShader);

	rc = GL_FALSE;
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &rc);
	if (rc != GL_TRUE) {
		ERR("Couldn't Compile Frag Shader %d!\n", fShader);
		Shader_ShaderLog(fShader);
		return -1;
	}

	glAttachShader(pShader, fShader);

	// linking
	glLinkProgram(pShader);

	rc = GL_FALSE;
	glGetProgramiv(pShader, GL_LINK_STATUS, &rc);
	if (rc != GL_TRUE) {
		ERR("Couldn't Compile Frag Shader %d!\n", fShader);
		Shader_ProgramLog(pShader);
		return -1;
	}

	return pShader;
}

/* Shader_ProgramLog : Prints the Log for the Shader */
s32 Shader_ProgramLog(GLint program)
{
	char *msg;
	int msg_len, msg_cap;

	if (!glIsProgram(program)) {
		ERR("Program '%d' isn't real!\n", program);
		return -1;
	}

	msg = NULL;
	msg_len = msg_cap = 0;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msg_cap);

	msg = calloc(sizeof(*msg), msg_cap);

	glGetProgramInfoLog(program, msg_cap, &msg_len, msg);

	if (msg_len > 0) {
		printf("%s\n", msg);
	}

	free(msg);

	return 0;
}

/* Shader_ShaderLog : prints the shader log */
s32 Shader_ShaderLog(GLuint shader)
{
	char *msg;
	int msg_len, msg_cap;

	if (!glIsShader(shader)) {
		ERR("Shader %d isn't a real program!\n", shader);
		return -1;
	}

	msg = NULL;
	msg_len = msg_cap = 0;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_cap);

	msg = calloc(sizeof(*msg), msg_cap);

	glGetShaderInfoLog(shader, msg_cap, &msg_len, msg);

	if (msg_len > 0) {
		printf("%s\n", msg);
	}

	free(msg);

	return 0;
}

