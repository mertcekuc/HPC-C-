#include <iostream>
#include <math.h>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>


#define DIMS 1000.0
#define LIM_RADIUS 10.0
#define N 100000
#define ITER_COOUNT 3
#define K 1.0
#define DT 0.01

typedef struct Body {
    double mass{1.0};
    double vx{0.0}, vy{0.0};
    double x,y;
} Body;


void calculate_interactions(Body &b, std::vector<Body> &arr){
    double dx, dy, distance;
    double f, fx{0}, fy{0};

    for(size_t i=0; i<N; i++){
        if(&b == &arr[i]) continue;

        dx = arr[i].x - b.x;
        dy = arr[i].y - b.y;
        distance = std::sqrt(dx*dx + dy*dy);
        
        if(distance > LIM_RADIUS || distance == 0.0) continue;
        
        f = K*(b.mass * arr[i].mass) / (distance*distance);
        fx += (f * dx / distance);
        fy += (f * dy / distance);

    }

        double ax{fx/b.mass}, ay{fy/b.mass};
        b.vx += ax * DT;
        b.vy += ay * DT;
}

void process_movements(std::vector<Body> &arr){
    for(size_t i = 0; i<N; i++){
        arr[i].x += arr[i].vx * DT;
        arr[i].y += arr[i].vy * DT;

        if(arr[i].x > DIMS){
            arr[i].x = DIMS;
            arr[i].vx *= -1;
        }

        else if(arr[i].x < 0){
            arr[i].x = 0;
            arr[i].vx *= -1;
        }

        if(arr[i].y > DIMS){
            arr[i].y = DIMS;
            arr[i].vy *= -1;
        }

        else if(arr[i].y < 0){
            arr[i].y = 0;
            arr[i].vy *= -1;
        }


    }
}

void process_movement(Body &b){
    b.x += b.vx * DT;
    b.y += b.vy * DT;

        if(b.x > DIMS){
            b.x = DIMS;
            b.vx *= -1;
        }

        else if(b.x < 0){
            b.x = 0;
            b.vx *= -1;
        }

        if(b.y > DIMS){
            b.y = DIMS;
            b.vy *= -1;
        }

        else if(b.y < 0){
            b.y = 0;
            b.vy *= -1;
        }
}

void initialize_particles(std::vector<Body> &arr){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, DIMS);

    for(size_t i=0; i<N; i++){
        arr[i].x = dist(gen);
        arr[i].y = dist(gen);
    }
}


int main(){

    std::vector<Body> particules (N);
    initialize_particles(particules);
    std::cout << "Starting simulation with " << N << " particules" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    omp_set_num_threads(4);
    
    #pragma omp parallel shared(particules)
    {
        int t_id = omp_get_thread_num();
        for(int iter=0; iter<ITER_COOUNT; iter++){

            #pragma omp for
            for (int i=0; i<N; i++){
                calculate_interactions(particules[i],particules);
            }

            #pragma omp for
            for (int i=0; i<N; i++){
                process_movement(particules[i]);
            }

            if(t_id == 0)
            std::cout << "Iteration " << iter+1 << " completed" << std::endl;


        }
    
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms" << std::endl;

    return 0;
}