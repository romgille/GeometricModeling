#pragma once
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;


glm::vec3 circumcenter(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
{
	double xba, yba, zba, xca, yca, zca, xda, yda, zda;
	double balength, calength, dalength;
	double xcrosscd, ycrosscd, zcrosscd;
	double xcrossdb, ycrossdb, zcrossdb;
	double xcrossbc, ycrossbc, zcrossbc;
	double denominator;
	double xcirca, ycirca, zcirca;

	/* Use coordinates relative to point `a' of the tetrahedron. */
	xba = p2.x - p1.x;
	yba = p2.y - p1.y;
	zba = p2.z - p1.z;
	xca = p3.x - p1.x;
	yca = p3.y - p1.y;
	zca = p3.z - p1.z;
	xda = p4.x - p1.x;
	yda = p4.y - p1.y;
	zda = p4.z - p1.z;
	/* Squares of lengths of the edges incident to `a'. */
	balength = xba * xba + yba * yba + zba * zba;
	calength = xca * xca + yca * yca + zca * zca;
	dalength = xda * xda + yda * yda + zda * zda;
	/* Cross products of these edges. */
	xcrosscd = yca * zda - yda * zca;
	ycrosscd = zca * xda - zda * xca;
	zcrosscd = xca * yda - xda * yca;
	xcrossdb = yda * zba - yba * zda;
	ycrossdb = zda * xba - zba * xda;
	zcrossdb = xda * yba - xba * yda;
	xcrossbc = yba * zca - yca * zba;
	ycrossbc = zba * xca - zca * xba;
	zcrossbc = xba * yca - xca * yba;

	denominator = 0.5 / (xba * xcrosscd + yba * ycrosscd + zba * zcrosscd);

	/* Calculate offset (from `a') of circumcenter. */
	xcirca = (balength * xcrosscd + calength * xcrossdb + dalength * xcrossbc) *
		denominator;
	ycirca = (balength * ycrosscd + calength * ycrossdb + dalength * ycrossbc) *
		denominator;
	zcirca = (balength * zcrosscd + calength * zcrossdb + dalength * zcrossbc) *
		denominator;

	return glm::vec3(xcirca + p1.x, ycirca + p1.y, zcirca + p1.z);
}


void computeVoronoiDelaunay(vector<glm::vec3> &input_points, vector<glm::vec3> & voronoi_points, vector<glm::uvec4> & delaunay_facets)
{
	delaunay_facets.clear();
	voronoi_points.clear();
	size_t tmp;

	ofstream fout("my_delaunay_in.txt");
	if (!fout.is_open())
	{
		cout << "Unable to open file my_delaunay_in.txt!\n";
		return;
	}

	fout << 3 << endl;
	fout << input_points.size() << endl;
	for (size_t i = 0;i < input_points.size();i++)
		fout << input_points[i].x << " " << input_points[i].y << " " << input_points[i].z << endl;
	fout.close();

	system("bin\\qhull\\qdelaunay.exe i Qz Qt < my_delaunay_in.txt > my_delaunay_out.txt");

	ifstream fin("my_delaunay_out.txt");
	if (!fin.is_open())
	{
		cout << "Unable to open file my_delaunay_out.txt!\n";
		return;
	}

	string s;
	fin >> tmp >> tmp;
	while (getline(fin, s))
	{
		std::stringstream myline(s);
		glm::uvec4 f;
		myline >> f[0] >> f[1] >> f[2] >> f[3];
		delaunay_facets.push_back(f);
	}
	fin.close();

	for (size_t i = 0; i < delaunay_facets.size(); ++i)
		voronoi_points.push_back(circumcenter(input_points[delaunay_facets[i][0]], input_points[delaunay_facets[i][1]],
			input_points[delaunay_facets[i][2]], input_points[delaunay_facets[i][3]]));

	remove("my_delaunay_in.txt");
	remove("my_delaunay_out.txt");
}

void rotate(glm::vec3 & inputvec, glm::vec3 rotation_axis, float theta, bool tonormalize)
{
	const float cos_theta = cos(theta);
	const float dot = glm::dot(inputvec, rotation_axis);
	glm::vec3 cross = glm::cross(inputvec, rotation_axis);

	inputvec *= cos_theta;
	inputvec += dot * (1.0f - cos_theta) * rotation_axis;
	inputvec -= sin(theta) * cross;

	if (tonormalize) inputvec = glm::normalize(inputvec);
}
