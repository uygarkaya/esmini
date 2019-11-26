/* 
 * esmini - Environment Simulator Minimalistic 
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * 
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#ifndef VIEWER_HPP_
#define VIEWER_HPP_

#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osgGA/NodeTrackerManipulator>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osgText/Text>
#include <osgAnimation/EaseMotion>
#include <string>

#include "RubberbandManipulator.hpp"
#include "RoadManager.hpp"
#include "CommonMini.hpp"

#define TRAIL_DOT_LIFE_SPAN 20.0  // seconds
#define TRAIL_DOT_FADE_DURATION 3.0  // seconds
#define TRAIL_MAX_DOTS 1000

namespace viewer
{
	class AlphaFadingCallback : public osg::StateAttributeCallback
	{
	public:
		AlphaFadingCallback(osgViewer::Viewer *viewer, osg::Vec4 color)
		{
			_motion = new osgAnimation::InCubicMotion(0.0f, TRAIL_DOT_FADE_DURATION);
			color_ = color;
			viewer_ = viewer;
			born_time_stamp_ = viewer_->elapsedTime();
			time_stamp_ = born_time_stamp_;
		}
		virtual void operator()(osg::StateAttribute*, osg::NodeVisitor*);
		void Reset() { _motion->reset(); }

	protected:
		osg::ref_ptr<osgAnimation::InCubicMotion> _motion;

	private:
		osg::Vec4 color_;
		double time_stamp_;
		double born_time_stamp_;
		osgViewer::Viewer *viewer_;
	};

	class TrailDot
	{
	public:
		osg::ref_ptr<osg::PositionAttitudeTransform> dot_;
		osg::ref_ptr<osg::Material> material_;
		float time_born;

		TrailDot(float time, double x, double y, double z, double heading,
			osgViewer::Viewer *viewer, osg::Group *parent, osg::ref_ptr<osg::Node> dot_node, osg::Vec4 trail_color);
		void Reset(float time, double x, double y, double z);

	private:
		AlphaFadingCallback *fade_callback_;
	};

	class Trail
	{
	public:
		TrailDot* dot_[TRAIL_MAX_DOTS];
		int n_dots_;
		int current_;
		osg::Group *parent_;
		osg::Node *dot_node_;
		void AddDot(float time, double x, double y, double z, double heading);

		Trail(osg::Group *parent, osgViewer::Viewer *viewer, osg::ref_ptr<osg::Node> dot_node, osg::Vec3 color) :
			parent_(parent), 
			viewer_(viewer),
			n_dots_(0), 
			current_(0),
			dot_node_(dot_node)
		{
			color_[0] = color[0];
			color_[1] = color[1];
			color_[2] = color[2];
		}

	private:
		osg::Vec4 color_;
		osgViewer::Viewer *viewer_;
	};

	class PointSensor
	{
	public:
		osg::ref_ptr<osg::PositionAttitudeTransform> ball_;
		double ball_radius_;
		osg::ref_ptr<osg::Geometry> line_;
		osg::ref_ptr<osg::Vec3Array> line_vertex_data_;

		PointSensor(): line_(0), line_vertex_data_(0), ball_(0) {}
	};

	class CarModel
	{
	public:
		osg::ref_ptr<osg::LOD> node_;
		osg::ref_ptr<osg::PositionAttitudeTransform> txNode_;
		std::vector<osg::ref_ptr<osg::PositionAttitudeTransform>> wheel_;
		osg::ref_ptr<osg::LOD> model;
		osg::Quat quat_;
		double size_x;
		double size_y;
		double center_x;
		double center_y;
		double wheel_angle_;
		double wheel_rot_;
		std::string filename_;
		PointSensor *speed_sensor_;
		PointSensor *road_sensor_;
		PointSensor *lane_sensor_;
		PointSensor *trail_sensor_;
		PointSensor *steering_sensor_;

		CarModel::CarModel(osgViewer::Viewer *viewer, osg::ref_ptr<osg::LOD> lod, osg::ref_ptr<osg::Group> parent, osg::ref_ptr<osg::Group> trail_parent, osg::ref_ptr<osg::Node> dot_node, osg::Vec3 trail_color);
		~CarModel();
		void SetPosition(double x, double y, double z);
		void SetRotation(double h, double p, double r);
		void UpdateWheels(double wheel_angle, double wheel_rotation);
		void CarModel::UpdateWheelsDelta(double wheel_angle, double wheel_rotation_delta);

		osg::ref_ptr<osg::PositionAttitudeTransform>  AddWheel(osg::ref_ptr<osg::Node> carNode, const char *wheelName);

		Trail *trail_;
		osgViewer::Viewer *viewer_;

	};

	class Viewer
	{
	public:
		int currentCarInFocus_;
		int camMode_;
		osg::ref_ptr<osg::Group> line_node_;

		// Vehicle position debug visualization
		osg::ref_ptr<osg::Node> shadow_node_;

		// Trail dot model
		osg::ref_ptr<osg::Node> dot_node_;

		// Road debug visualization
		osg::ref_ptr<osg::Group> odrLines_;
		osg::ref_ptr<osg::PositionAttitudeTransform> envTx_;
		osg::ref_ptr<osg::Node> environment_;
		osg::ref_ptr<osgGA::RubberbandManipulator> rubberbandManipulator_;
		osg::ref_ptr<osgGA::NodeTrackerManipulator> nodeTrackerManipulator_;
		std::vector<CarModel*> cars_;
		float lodScale_;
		osgViewer::Viewer *osgViewer_;
		osg::MatrixTransform* rootnode_;
		osg::Group* sensors_;
		osg::Group* trails_;
		roadmanager::OpenDrive *odrManager_;
		bool showInfoText;

		osg::ref_ptr<osg::Camera> infoTextCamera;
		osg::ref_ptr<osgText::Text> infoText;

		Viewer(roadmanager::OpenDrive *odrManager, const char *modelFilename, const char *scenarioFilename, osg::ArgumentParser arguments, bool create_ego_debug_lines = false);
		~Viewer();
		CarModel* AddCar(std::string modelFilepath, bool transparent, osg::Vec3 trail_color);
		int AddEnvironment(const char* filename);
		osg::ref_ptr<osg::LOD> LoadCarModel(const char *filename);
		void UpdateSensor(PointSensor *sensor, roadmanager::Position *pivot_pos, double target_pos[3]);
		void UpdateRoadSensors(PointSensor *road_sensor, PointSensor *lane_sensor, roadmanager::Position *pos);
		void setKeyUp(bool pressed) { keyUp_ = pressed; }
		void setKeyDown(bool pressed) { keyDown_ = pressed; }
		void setKeyLeft(bool pressed) { keyLeft_ = pressed; }
		void setKeyRight(bool pressed) { keyRight_ = pressed; }
		bool getKeyUp() { return keyUp_; }
		bool getKeyDown() { return keyDown_; }
		bool getKeyLeft() { return keyLeft_; }
		bool getKeyRight() { return keyRight_; }
		void SetQuitRequest(bool value) { quit_request_ = value; }
		bool GetQuitRequest() { return quit_request_;  }
		std::string getScenarioDir() { return scenarioDir_; }
		void SetInfoTextProjection(int width, int height);
		void SetInfoText(const char* text);
		void ShowInfoText(bool show);
		PointSensor* CreateSensor(int color[], bool create_ball, bool create_line, double ball_radius, double line_width);
		bool CreateRoadSensors(CarModel *vehicle_model);

	private:

		std::string scenarioDir_;

		bool CreateRoadLines(roadmanager::OpenDrive* od, osg::Group* parent);
		bool keyUp_;
		bool keyDown_;
		bool keyLeft_;
		bool keyRight_;
		bool quit_request_;
	};

	class ViewerEventHandler : public osgGA::GUIEventHandler
	{
	public:
		ViewerEventHandler(Viewer *viewer) : viewer_(viewer) {}
		bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&);

	private:
		Viewer * viewer_;
	};
}



#endif  // VIEWER_HPP_

