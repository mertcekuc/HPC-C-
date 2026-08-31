#include <iostream>
#include <math.h>
#include <vector>
#include <random>


#define DIMS 1000.0
#define LIM_RADIUS 10.0
#define N 100000
#define ITER_COOUNT 3
#define K 1.0

typedef struct Body {
    double mass{1.0};
    double vx{0.0}, vy{0.0};
    double x,y;
} Body;


void calculate_interactions(Body &b, std::vector<Body> &arr){
    double dx, dy, distance;
    double f, fx{0}, fy{0};

    for(size_t i=0; i<N; i++){
        dx = arr[i].x - b.x;
        dy = arr[i].y - b.y;
        distance = std::sqrt(std::pow(std::abs(dx),2)+
                    std::pow(std::abs(dy),2));
        
        if(distance > 10) continue;
        
        f = K*(b.mass * arr[i].mass) / (distance*distance);
        fx += (f * dx / distance);
        fy += (f * dy / distance);

    }

        double ax{fx/b.mass}, ay{fy/b.mass};
        b.vx += ax;
        b.vy += ay;
}

void process_movements(std::vector<Body> &arr){
    for(size_t i = 0; i<N; i++){
        arr[i].x += arr[i].vx;
        arr[i].y += arr[i].vy;

        if(arr[i].x > 1000){
            arr[i].x = 1000;
            arr[i].vx *= -1;
        }

        else if(arr[i].x < 0){
            arr[i].x = 0;
            arr[i].vx *= -1;
        }

        if(arr[i].y > 1000){
            arr[i].y = 1000;
            arr[i].vy *= -1;
        }

        else if(arr[i].y < 0){
            arr[i].y = 0;
            arr[i].vy *= -1;
        }


    }
}

void fill_grid(std::vector<Body> &arr){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1000.0);

    for(size_t i=0; i<N; i++){
        arr[i].x = dist(gen);
        arr[i].y = dist(gen);
    }
}


int main(){

    std::vector<Body> particules (N);
    fill_grid(particules);
    std::cout << "Starting simulation with " << N << " particules" << std::endl;
    double start = clock();

    for(int i=0; i<ITER_COOUNT; i++){
        for(size_t j=0; j<N; j++)
            calculate_interactions(particules[j],particules);
        
        process_movements(particules);
        std::cout << "Iteration " << i+1 << " completed" << std::endl;
    }

    double end = clock();
    std::cout << "Execution time: " << (end-start)/CLOCKS_PER_SEC << std::endl;

    return 0;
}