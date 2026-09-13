#include <iostream>
#include <math.h>
#include <vector>
#include <random>
#include <chrono>
#include <mpi.h>

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
    MPI_Init(NULL,NULL);
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    std::vector<Body> particules (N);
    std::vector<Body> local_grid;
    std::vector<std::vector<Body>> grids (4);
    int grid_dims = DIMS/std::sqrt(size);

    if(rank==0) {
        initialize_particles(particules);
        int row,col,grid;
        for(auto i:particules){
            col = i.x/grid_dims;
            row = i.y/grid_dims;
            grid = col * grid_dims + row;
            grids[grid].push_back(i);
        }
        for(int i=1; i<size; i++){
            int size = grids.size();
            MPI_Send(grids[i].data(),size, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }

    }
    else{
        MPI_Probe(0,0,MPI_COMM_WORLD, &status);
        int size;
        MPI_Get_count(&status, MPI_BYTE,&size);
        local_grid.resize(size);
        MPI_Recv(local_grid.data(), size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);

    }



    std::cout << "Starting simulation with " << N << " particules" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms" << std::endl;

    return 0;
}