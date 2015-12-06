#include "Camera.hpp"

Camera::Camera(Vec3<float> const &position, Vec3<float> const &lookAt)
{
	_theta = 0.0f;
	_phi = 0.0f;
	_speed = 0.0005f;
	_sensitivity = 0.05f;
	this->position = position;
	this->lookAt = lookAt;
	set();
}

Camera::Camera(Camera const &src)
{

}

Camera::~Camera(void)
{

}

void
Core::set(void)
{
	Mat4<float>		translation;

	up.set(0.0f, 1.0f, 0.0f);
	forward.set(lookAt - pos);
	forward.normalize();
	right.crossProduct(forward, up);
	right.normalize();
	up.crossProduct(right, forward);
	up.normalize();
	setView(view, forward, right, up);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

void
Core::move(float const &xrel, float const &yrel)
{
	_theta -= xrel * _sensitivity;
	_phi -= yrel * _sensitivity;
}

void
Core::setView(void)
{
	/*
	rx		ux		-dx		0
	ry		uy		-dy		0
	rz		uz		-dz		0
	0		0		0		1
	*/
	// first column
	view[0] = right.x;
	view[4] = right.y;
	view[8] = right.z;
	view[12] = 0.0f;
	// second column
	view[1] = up.x;
	view[5] = up.y;
	view[9] = up.z;
	view[13] = 0.0f;
	// third column
	view[2] = -forward.x;
	view[6] = -forward.y;
	view[10] = -forward.z;
	view[14] = 0.0f;
	// fourth column
	view[3] = 0.0f;
	view[7] = 0.0f;
	view[11] = 0.0f;
	view[15] = 1.0f;
}
