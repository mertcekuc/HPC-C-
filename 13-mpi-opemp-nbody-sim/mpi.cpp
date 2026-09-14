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


void calculate_interactions(Body &b, const std::vector<Body> &arr, const std::vector<std::vector<Body>> &halos){
    double dx, dy, distance;
    double f, fx{0}, fy{0};
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for(size_t i=0; i<arr.size(); i++){
        if(&b == &arr[i]) continue;

        dx = arr[i].x - b.x;
        dy = arr[i].y - b.y;
        distance = std::sqrt(dx*dx + dy*dy);
        
        if(distance > LIM_RADIUS || distance == 0.0) continue;
        
        f = K*(b.mass * arr[i].mass) / (distance*distance);
        fx += (f * dx / distance);
        fy += (f * dy / distance);

    }
    for(int i = 0; i<size; i++){
        for(size_t j= 0; j< halos[i].size();j++){

            dx = halos[i][j].x - b.x;
            dy = halos[i][j].y - b.y;
            distance = std::sqrt(dx*dx + dy*dy);
            
            if(distance > LIM_RADIUS || distance == 0.0) continue;
            
            f = K*(b.mass * halos[i][j].mass) / (distance*distance);
            fx += (f * dx / distance);
            fy += (f * dy / distance);
        }
    }

        double ax{fx/b.mass}, ay{fy/b.mass};
        b.vx += ax * DT;
        b.vy += ay * DT;
}

void process_movements(std::vector<Body> &arr){
    for(size_t i = 0; i<arr.size(); i++){
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

void find_halos(const std::vector<Body> &local_grid, std::vector<Body> &local_halos){
    local_halos.clear();
    int rank,size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int grid_size = DIMS/std::sqrt(size);;
    int space_dims = std::sqrt(size);
    int col{rank%space_dims};
    int row{rank/space_dims};

    int x_start{col*grid_size}, x_end{(col+1)*grid_size};
    int y_start{row*grid_size}, y_end{(row+1)*grid_size};

    for(auto &i: local_grid){
        if((i.x<=x_start+LIM_RADIUS && i.x>=x_start) 
            || (i.x >= x_end - LIM_RADIUS && i.x <= x_end))
                local_halos.push_back(i);
        else if((i.y<=y_start+LIM_RADIUS && i.y>=y_start) 
            || (i.y >= y_end - LIM_RADIUS && i.y <= y_end))
                local_halos.push_back(i);

    }


}

void find_grids(const std::vector<Body> &local, std::vector<std::vector<Body>> &grids){
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int space_dimm = std::sqrt(size);
    int grid_dimm = DIMS/std::sqrt(size);

    for(int i=0; i<size; i++) grids[i].clear();
    int row,col,rank;
    for(auto &i : local){
        col = i.x / grid_dimm;
        row = i.y / grid_dimm;
        rank = row * space_dimm + col;
        grids[rank].push_back(i); 
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
    std::vector<Body> local_halo;
    std::vector<std::vector<Body>> grids (size);
    std::vector<std::vector<Body>> halos (size);

    int grid_dims = DIMS/std::sqrt(size);
    int space_dims = std::sqrt(size);
    if(rank==0) {
        initialize_particles(particules);
        int row,col,grid;
        for(auto i:particules){
            col = i.x/grid_dims;
            row = i.y/grid_dims;
            grid = row * space_dims + col;
            grids[grid].push_back(i);
        }
        for(int i=1; i<size; i++){
            int size = grids[i].size();
            MPI_Send(grids[i].data(),size * sizeof(Body), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        local_grid.assign(grids[0].begin(), grids[0].end());
        particules.clear();
        
    }
    else{
        MPI_Probe(0,0,MPI_COMM_WORLD, &status);
        int size;
        MPI_Get_count(&status, MPI_BYTE,&size);
        local_grid.resize(size/sizeof(Body));
        MPI_Recv(local_grid.data(), size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);

    }

    std::cout << "Starting simulation with " << N << " particules" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i=1; i<=ITER_COOUNT; i++){
        find_halos(local_grid, local_halo);
        for(int j=0; j<size; j++){
            int size_msg;
            if(j==rank){
                size_msg = local_halo.size();
                MPI_Bcast(&size_msg,1,MPI_INT,rank,MPI_COMM_WORLD);
                MPI_Bcast(local_halo.data(),size_msg * sizeof(Body),MPI_BYTE,rank,MPI_COMM_WORLD);
                }
                else{
                    MPI_Bcast(&size_msg,1,MPI_INT,j,MPI_COMM_WORLD);
                    halos[j].resize(size_msg);
                    MPI_Bcast(halos[j].data(),size_msg * sizeof(Body),MPI_BYTE,j,MPI_COMM_WORLD);
                }
            }
        for(auto &b:local_grid){
            calculate_interactions(b, local_grid, halos);
        }
        process_movements(local_grid);
        find_grids(local_grid,grids);
        //share grids
        {
        int message_counts[size];
        int msg;
        int dists[size];

        for(int j=0; j<size; j++){
                int msg = grids[j].size();
                MPI_Gather(&msg,1,MPI_INT,message_counts,1,MPI_INT,j,MPI_COMM_WORLD);
                if(j==rank){
                    dists[0]=0;
                    for(int index=1;index<size; index++){
                        dists[index]= dists[index-1] + message_counts[index-1];
                    }
                }
                MPI_Gatherv(grids[j].data(),msg * sizeof(Body),MPI_BYTE,&local_grid,message_counts,dists,MPI_BYTE,j,MPI_COMM_WORLD);
                }

        }
        }
    
    

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms" << std::endl;

    return 0;
}