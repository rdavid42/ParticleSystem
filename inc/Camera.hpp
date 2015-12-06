
#ifndef CAMERA_HPP
# define CAMERA_HPP

# include "Vec3.hpp"
# include "Mat4.hpp"

class Camera
{
public:
	Mat4<float>		view;

	Vec3<float>		position;
	Vec3<float>		forward;
	Vec3<float>		right;
	Vec3<float>		up;
	Vec3<float>		lookAt;

	Camera(void);
	Camera(Camera const &src);
	~Camera(void);

private:
	float			_speed;
	float			_sensitivity;
	float			_theta;
	float			_phi;

	void			set(void);
	void			setView(void);
};

#endif
