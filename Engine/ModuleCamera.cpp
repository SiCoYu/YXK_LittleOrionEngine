#include "Globals.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleWindow.h"
#include "SDL.h"

ModuleCamera::ModuleCamera()
{
}

// Destructor
ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::Init()
{
	int windowWidth, windowHeight;
	SDL_GetWindowSize(App->window->window, &windowWidth, &windowHeight);
	
	aspect_ratio = (float)windowWidth / windowHeight;
	pitch_angle = 0;

	// CREATES PROJECTION MATRIX
	camera_frustum.type = FrustumType::PerspectiveFrustum;
	camera_frustum.pos = float3::zero;
	camera_frustum.front = -float3::unitZ;
	camera_frustum.up = float3::unitY;
	camera_frustum.nearPlaneDistance = 1.f;
	camera_frustum.farPlaneDistance = 100.0f;
	camera_frustum.verticalFov = math::pi / 4.0f;
	camera_frustum.horizontalFov = 2.f * atanf(tanf(camera_frustum.verticalFov * 0.5f) * aspect_ratio);

	generateMatrices();

	return true;
}

update_status ModuleCamera::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleCamera::Update()
{

	return UPDATE_CONTINUE;
}

update_status ModuleCamera::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleCamera::CleanUp()
{
	return true;
}

void ModuleCamera::SetFOV(const float fov)
{
	camera_frustum.verticalFov = fov;
	camera_frustum.horizontalFov = 2.f * atanf(tanf(camera_frustum.verticalFov * 0.5f) * aspect_ratio);

	generateMatrices();
}

void ModuleCamera::SetAspectRatio(const float aspect_ratio)
{
	this->aspect_ratio = aspect_ratio;
	camera_frustum.horizontalFov = 2.f * atanf(tanf(camera_frustum.verticalFov * 0.5f) * aspect_ratio);

	generateMatrices();
}

void ModuleCamera::SetNearDistance(const float distance)
{
	camera_frustum.nearPlaneDistance = distance;

	generateMatrices();
}

void ModuleCamera::SetFarDistance(const float distance)
{
	camera_frustum.farPlaneDistance = distance;

	generateMatrices();
}

void ModuleCamera::SetPosition(const float3 position)
{
	camera_frustum.pos = position;

	generateMatrices();
}

void ModuleCamera::SetOrientation(const float3 orientation)
{

	generateMatrices();
}

void ModuleCamera::LookAt(const float x, const float y, const float z)
{
	generateMatrices();

}

void ModuleCamera::MoveUp(const float distance)
{
	float3 new_camera_pos = camera_frustum.pos;
	new_camera_pos.y = new_camera_pos.y + distance;
	camera_frustum.pos = new_camera_pos;

	generateMatrices();
}

void ModuleCamera::MoveDown(const float distance)
{
	float3 new_camera_pos = camera_frustum.pos;
	new_camera_pos.y = new_camera_pos.y - distance;
	camera_frustum.pos = new_camera_pos;

	generateMatrices();
}

void ModuleCamera::MoveFoward(const float distance)
{
	camera_frustum.pos = camera_frustum.pos + camera_frustum.front.ScaledToLength(distance);

	generateMatrices();
}

void ModuleCamera::MoveBackward(const float distance)
{
	camera_frustum.pos = camera_frustum.pos - camera_frustum.front.ScaledToLength(distance);

	generateMatrices();
}

void ModuleCamera::MoveLeft(const float distance)
{
	camera_frustum.pos = camera_frustum.pos - camera_frustum.WorldRight().ScaledToLength(distance);

	generateMatrices();
}

void ModuleCamera::MoveRight(const float distance)
{
	camera_frustum.pos = camera_frustum.pos + camera_frustum.WorldRight().ScaledToLength(distance);

	generateMatrices();
}

void ModuleCamera::RotatePitch(const float angle)
{
	if (pitch_angle + angle >= math::pi/2 || pitch_angle + angle <= -math::pi / 2) {
		return;
	}
	pitch_angle += angle;
	float3x3 rotation_matrix = float3x3::identity;
	rotation_matrix.SetRotatePart(camera_frustum.WorldRight(), angle);
	camera_frustum.up = rotation_matrix * camera_frustum.up;
	camera_frustum.front = rotation_matrix * camera_frustum.front;

	generateMatrices();
}

void ModuleCamera::RotateYaw(const float angle)
{
	float3x3 rotation_matrix = float3x3::RotateY(angle);
	camera_frustum.up = rotation_matrix * camera_frustum.up;
	camera_frustum.front = rotation_matrix * camera_frustum.front;

	generateMatrices();
}


void ModuleCamera::generateMatrices()
{
	proj = camera_frustum.ProjectionMatrix();
	view = camera_frustum.ViewMatrix();
}