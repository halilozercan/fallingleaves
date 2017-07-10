#include "fallinfo.h"

FallInfo::FallInfo() {
  this->id = 0;
  this->duration = 0;
}

FallInfo::FallInfo(int id) {
  this->id = id;
  this->duration = 0;
}

void FallInfo::forgeTrajectory() {
  if(this->id == STEADY_FALL){
  	angular.push_back(std::make_pair(10.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
  	velocity.push_back(glm::vec3(0.0f, -0.2f, 0.0f));
  }
  else if(this->id == FLUTTER_FALL){
  	angular.push_back(std::make_pair(10.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
  	std::vector<float> velocity_x (flutter_vx_array, flutter_vx_array + sizeof(flutter_vx_array) / sizeof(flutter_vx_array[0]) );
  	std::vector<float> velocity_y (flutter_vy_array, flutter_vy_array + sizeof(flutter_vy_array) / sizeof(flutter_vy_array[0]) );
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(velocity_x[i]/200, velocity_y[i]/100, 0.0f));
  	}
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(velocity_x[i]/200, velocity_y[i]/100, 0.0f));
  	}
  }
  else if(this->id == TUMBLE_FALL){
  	angular.push_back(std::make_pair(10.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
  	std::vector<float> velocity_x (flutter_vx_array, flutter_vx_array + sizeof(flutter_vx_array) / sizeof(flutter_vx_array[0]) );
  	std::vector<float> velocity_y (flutter_vy_array, flutter_vy_array + sizeof(flutter_vy_array) / sizeof(flutter_vy_array[0]) );
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(velocity_x[i]/200, velocity_y[i]/100, 0.0f));
  	}
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(-(velocity_x[i]/200), velocity_y[i]/100, 0.0f));
  	}
  }
  else if(this->id == SPIRAL_FALL){
  	angular.push_back(std::make_pair(10.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
  	std::vector<float> velocity_x (spiral_vx_array, spiral_vx_array + sizeof(spiral_vx_array) / sizeof(spiral_vx_array[0]) );
  	std::vector<float> velocity_y (spiral_vy_array, spiral_vy_array + sizeof(spiral_vy_array) / sizeof(spiral_vy_array[0]) );
  	std::vector<float> velocity_z (spiral_vz_array, spiral_vz_array + sizeof(spiral_vz_array) / sizeof(spiral_vz_array[0]) );
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(velocity_x[i]/10, velocity_y[i]/100, velocity_z[i]/10));
  	}
  }
  else if(this->id == CHAOTIC_FALL){
  	angular.push_back(std::make_pair(10.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
  	std::vector<float> velocity_x (chaotic, chaotic + sizeof(chaotic) / sizeof(chaotic[0]) );
  	std::vector<float> velocity_y (spiral_vy_array, spiral_vy_array + sizeof(spiral_vy_array) / sizeof(spiral_vy_array[0]) );
  	std::vector<float> velocity_z (chaotic, chaotic + sizeof(chaotic) / sizeof(chaotic[0]) );
  	for(int i=0;i<velocity_x.size();i++){
  		velocity.push_back(glm::vec3(velocity_x[i]/10, velocity_y[i]/100, -velocity_z[i]/10));
  	}
  }
}