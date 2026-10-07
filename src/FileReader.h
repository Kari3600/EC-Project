#ifndef FILEREADER_H
#define FILEREADER_H

#include "Problem.h"

#include <array>
#include <istream>
#include <vector>

std::vector<std::array<int, 3>> readCSV(std::istream& str);
Problem readProblem(std::istream& str);

#endif