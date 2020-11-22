#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include<vector>
#include<stack>
#include"Floor_Cleaning_Robot_v4.hpp"
using namespace std;

// enum tile_type {floor = 0, wall = 1, recharge = 2, unknown = 3};
// const int direction[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
tile_map* TileMap;
int max_BFS;

int CharToType(char c){
    switch(c){
        case '0': return floor;
        case '1': return wall;
        case 'R': return recharge;
        default : return unknown;
    }
}

void set_maxBFS(float t){
    if(t<0.1) max_BFS = 500;
    else if(t < 0.5) max_BFS = 250;
    else if(t< 0.75) max_BFS = 150;
    else if(t < 1) max_BFS = 80;
    else if(t < 2) max_BFS = 50;
    else if(t < 3) max_BFS = 20;
    else max_BFS = 0;
}

//class trio
trio::trio(position p, int s, vector<position> v){
    pos = p;
    step = s;
    related = v;
}


//class tile
tile::tile(int t, int row, int col){
    set(t,row,col);
}
void tile::set(int t, int row, int col){
    minstep = -1;
    cleaned = false;
    type = t;
    rounded_visited = -1;
    search_visited = -1;
    pos = position(row,col);
}
void tile::clean(){
    if(cleaned){
        printf("Error: tile clean more than once\n");
        exit(7);
    }
    cleaned = true;
}


//class tile_map
tile_map::tile_map(ifstream infile){
    char temp;
    infile >> rows >> cols >> B;
    Rpos.col = Rpos.row = -1;
    search_id = 0;
    round_id = 0;
    map = new tile*[rows];
    for(int i=0;i<rows;i++){
        map[i] = new tile[cols];
        for(int j=0;j<cols;j++){
            infile >> temp;
            temp = CharToType(temp);
            if(temp == unknown){
                printf("Error: unknown tile type\n");
                exit(2);
            }
            else if(temp == recharge){
                if(Rpos.col + Rpos.row != -2){
                    printf("Error: multipe charges\n");
                    exit(3);
                }
                else{
                    Rpos.row = i;
                    Rpos.col = j;
                }
            }
            map[i][j].set(temp,i,j);
        }
    }
    calculate_minstep();
}
bool tile_map::is_walkable(position pos){
    int r = pos.row;
    int c = pos.col;
    if(r<0 || r>=rows) return false;
    if(c<0 || r>=cols) return false;
    if(map[r][c].type == wall) return false;
    return true;
}

void tile_map::calculate_minstep(){
    queue<trio> Q;
    trio tp;
    vector<position> vp;
    while(!Q.empty()) Q.pop();
    Q.push(trio(Rpos,0));
    get_tile(Rpos,"root clean").cleaned = true;
    while(!Q.empty()){
        tp = Q.front();
        vp = tp.related;
        vp.push_back(tp.pos);
        if(get_tile(tp.pos,"calculate_minstep3").minstep == -1){
            get_tile(tp.pos,"calculate_minstep1").minstep = tp.step;
            get_tile(tp.pos,"calculate_minstep2").related = vp;
            for(int i=0;i<4;i++){
                if(is_walkable(tp.pos.stepwise_move(i)) && get_tile(tp.pos.stepwise_move(i),"calculate_minstep4").minstep==-1){
                    Q.push(trio(tp.pos.stepwise_move(i),tp.step+1,vp));
                }
            }
            todo.push(tp.pos);
        }
        Q.pop();
        // printf("h\n");
    }
}
tile& tile_map::get_tile(position p, const char* origin){
    if(p.row<0 || p.row>=rows || p.col<0 || p.col>=cols){
        printf("Error: get_tile failed, from %s\n", origin);
        exit(6);
    }
    // printf("get_tile(%d,%d):%d\n", p.row, p.col, map[p.row][p.col].minstep);
    return map[p.row][p.col];
}
trio tile_map::find_uncleaned(position init, int battery){
    queue<trio> Q;
    trio tp;
    trio best;
    vector<position> vp;
    tile tt;
    int count = 0;
    search_id++;
    while(!Q.empty()) Q.pop();
    for(int i=0;i<4;i++) if(is_walkable(init.stepwise_move(i))) Q.push(trio(init.stepwise_move(i),1));
    // Q.push(trio(init,0)); 
    get_tile(Rpos,"root clean").search_visited = search_id;
    while(!Q.empty() && count++<max_BFS){
        tp = Q.front();
        if(best.step != -1 && best.step < tp.step) break;
        vp = tp.related;
        vp.push_back(tp.pos);
        tt = get_tile(tp.pos,"find_uncleaned1");
        if(tt.search_visited != search_id && tp.step + tt.minstep < battery-1){
            if(tt.cleaned){
                for(int i=0;i<4;i++){
                    if(is_walkable(tp.pos.stepwise_move(i))){
                        Q.push(trio(tp.pos.stepwise_move(i),tp.step+1,vp));
                    }
                }
            }
            else{
                if(best.step == -1 || tile_compare(tt,get_tile(best.pos,"find_uncleaned2"))){
                    best = tp;
                    best.related.push_back(tp.pos);
                }
            }
        }
        tt.search_visited = search_id;
        Q.pop();
    }
    // printf("#hop track size:%ld\n",best.related.size());
    return best;
}
void tile_map::print_out(int t){
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            if(t == 1) printf("%d ", map[i][j].type);
            else if(t == 2) printf("%3d ", map[i][j].minstep);
            else{
                if(is_walkable(position(i,j))) printf("%c ", map[i][j].cleaned? '.' : '+');
                else printf("M ");
            }
        }
        printf("\n");
    }
}


//class robot
robot::robot(){
    pos = Rpos = TileMap->Rpos;
    maxbattery = battery = TileMap->B;
    while(!footprint.empty()) footprint.pop();
    // footprint.push(Rpos);
}
void robot::jump(){
    tile_map &tm = *TileMap;
    if(tm.todo.empty()){
        printf("Error: jump when todo stack empty\n");
        exit(8);
    }
    tile &target = tm.get_tile(tm.todo.top(),"robot jump2");
    tm.todo.pop();
    for(position p:target.related){
        footprint.push(p);
        if(!tm.get_tile(p,"robot jump3").cleaned) tm.get_tile(p,"robot jump3").clean();
    }
    pos = target.pos;
    battery -= (target.related.size() - 1);
}
void robot::walk(){
    tile_map &tm = *TileMap;
    position best, tpos;
    //找到適合的移動方向
    for(int i=0;i<4;i++){
        tpos = position(pos.row+direction[i][0], pos.col+direction[i][1]);
        if(tm.is_walkable(tpos) && tm.get_tile(tpos,"robot walk1").minstep <= battery-1){
            if(best.row == -1 || tile_compare(tm.get_tile(tpos,"robot walk2"), tm.get_tile(best,"robot walk3"))){
                best = tpos;
            }
        }
    }
    if(best.row == -1){
        printf("Error: robot cannot move, battery=%d\n", battery);
        exit(4);
    }
    //移動至最好的位置
    pos = best;
    if(best != Rpos) footprint.push(best);
    if(!tm.get_tile(best,"robot walk4").cleaned) tm.get_tile(best,"robot walk4.5").clean();
    //減少電量
    if(is_on_recharge()) battery = maxbattery;
    else battery--;
    if(battery <= 0){
        printf("Error: robot battery exhausted\n");
        exit(5);
    }
}
void robot::hop(){
    tile_map& tm = *TileMap;
    trio best;
    best = tm.find_uncleaned(pos,battery);
    if(best.step == -1) {
        walk();
    }
    else{
        tm.get_tile(best.pos,"robot hop2").clean();
        for(int i=0;i<best.related.size();i++){
            footprint.push(best.related[i]);
        }
        pos = best.pos;
        battery -= (best.related.size());
    }
}
bool tile_compare(const tile& a, const tile& b){
    int target_distance = (TileMap->todo.empty())? 0 : TileMap->get_tile(TileMap->todo.top(),"tile compare1").minstep;
    if(a.cleaned != b.cleaned) return !a.cleaned;
    else if(abs(target_distance - a.minstep) != abs(target_distance - b.minstep)) return (abs(target_distance - a.minstep) < abs(target_distance - b.minstep));
    else return false;
}
bool robot::is_on_recharge(){
    return pos == Rpos;
}
void robot::print_out(ofstream& ofs){
    int length = footprint.size() - 1l;
    if(length == 1) length = 0;
    ofs << length << endl;
    while(!footprint.empty()){
        ofs << footprint.front().row << " " << footprint.front().col << endl;
        footprint.pop();
    }
}

void DEBUG(robot &R){
    string d;
    static int count = 0;
    static bool stop = false;
    static int maxround;
    printf("(%d,%d), battery=%d, footprintsize=%ld, todosize=%ld\n", R.pos.row, R.pos.col,\
     R.battery, R.footprint.size(), TileMap->todo.size());
    if(!stop) cin >> d;
    // if(!stop) d = "a";
    else{
        if(maxround > count){
            count++;
        }
        else{
            stop = false;
            maxround = 0;
            count = 0;
            TileMap->print_out(3);
        }
    }

    if(d == "a"){
        stop = true;
        maxround = 10;
        count = 0;
    }
    else if(d == "b"){
        stop = true;
        maxround = 100;
        count = 0;
    }
    else if(d == "c"){
        stop = true;
        maxround = 1000;
        count = 0;
    }
    else if(d == "p") TileMap->print_out(3);
}

int main(int argc, char *argv[]){
    string debug;
    start = clock();
    if(argc != 2) {
        printf("Error: amount of file in file out is wrong\n");
        exit(1);
    }
    // ofstream outfile("final.path",ios::out);
    TileMap = new tile_map(ifstream(argv[1],ios::in));
    robot* Robot = new robot();
    TileMap->print_out(2);
    printf("#build time: %.3fs, \n", float(clock() - start)/CLOCKS_PER_SEC);
    set_maxBFS(float(clock() - start)/CLOCKS_PER_SEC);
    while(!TileMap->todo.empty()){
        last = clock();
        Robot->jump();
        while(!Robot->is_on_recharge()) {
            Robot->hop();
            while(!TileMap->todo.empty() && TileMap->get_tile(TileMap->todo.top(),"robot jump1").cleaned) TileMap->todo.pop();
            // printf("#hop:(%d,%d)\n", Robot->pos.row, Robot->pos.col);
        }
        // printf("\n");
        DEBUG(*Robot);
        printf("round time: %.3lfs\n", float(clock()-last)/CLOCKS_PER_SEC);
    }
    Robot->footprint.push(Robot->Rpos);
    TileMap->print_out(3);
    Robot->print_out(outfile);
    printf("time: %.3lfs\n", double(clock() - start)/CLOCKS_PER_SEC);
}