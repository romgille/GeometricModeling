#pragma once
#include <glm/glm.hpp>
#include <vector>


glm::vec3 circumcenter(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4); 

void computeVoronoiDelaunay(std::vector<glm::vec3> &input_points, std::vector<glm::vec3> & voronoi_points, std::vector<glm::uvec4> & delaunay_facets);

void rotate(glm::vec3 & inputvec, glm::vec3 rotation_axis, float theta, bool tonormalize = false);