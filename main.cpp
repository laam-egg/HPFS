#include <bits/stdc++.h>

using namespace std;

const int SUNLIGHT = 1;
const int NO_SUNLIGHT = 0;
pair<int, pair<int, int>> CropPrice;

class CSVRow
{
    public:
        std::string operator[](std::size_t index) const
        {
            return std::string(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
        }
        std::size_t size() const
        {
            return m_data.size() - 1;
        }
        void readNextRow(std::istream& str)
        {
            std::getline(str, m_line);
            m_data.clear();
            m_data.emplace_back(-1);
            std::string::size_type pos = 0;
            while((pos = m_line.find(',', pos)) != std::string::npos)
            {
                m_data.emplace_back(pos);
                ++pos;
            }
            // This checks for a trailing comma with no data after it.
            pos   = m_line.size();
            m_data.emplace_back(pos);
        }
    private:
        std::string         m_line;
        std::vector<int>    m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}


int I, R, T, H, Col, Row;
vector <string> crop_name;
vector <string> family_name;
vector <int> cultivate_time;
vector <double> survival_rate;
vector <int> multiple_harvestime;
vector <vector<double>> prices;
vector <double> harvested_amount;
vector <bool> sunlight_requirement;
vector<vector<int> > rack_conf;
vector<int> predefined_conf
= { 1,0,0,1,0,0,0,1,0,1,0,1,1,0,0,0,0,0 };

vector <vector<double>> U;
vector <vector<int>> adj;

vector <double>  W(R);
vector <vector<double>> P(I, vector<double>(T));

vector <double> Q(I);

vector <vector<int>> F(H);
vector <vector<int>> schedule(R, vector<int>(T));
vector<int> rack_num;


class Cultivation {
public:
    double price;
    int crop_id;
    int harvest_week;

    Cultivation(int price, int crop_id, int harvest_week) {
        this->price = price;
        this->crop_id = crop_id;
        this->harvest_week = harvest_week;
    }

    int start_week() {
        return harvest_week - cultivate_time[crop_id] + 1;
    }
};


vector <Cultivation> cultivations;
vector <int> shortest_cultivation;
vector <vector<Cultivation>> rack_cultivations;
vector <vector<int>> rack_timeslot;

vector <int> avail_slot;

double revenue;
double best_revenue = 0;


void read_crop_data() {
    ifstream file("c:\\Users\\kurod\\Documents\\CPP\\HPFS\\dai_crop_data.csv");
    CSVRow row;
    while(file >> row){
        crop_name.push_back(row[1]);
        family_name.push_back(row[2]);
        cultivate_time.push_back(stoi(row[3]));
        sunlight_requirement.push_back(stoi(row[4]));
        harvested_amount.push_back(stod(row[5]));
        multiple_harvest_time.push_back(stoi(row[6]));
    }
    I = crop_name.size();
}

void read_price_data() {
    ifstream file("c:\\Users\\kurod\\Documents\\CPP\\HPFS\\dai_prices.csv");
    CSVRow row;
    vector<vector<double> > init_prices(I, vector<double>());
    prices = init_prices;
    while (file >> row) {
        for(int i = 0; i < I; ++i) {
            prices[i].push_back(stod(row[i]));
        }
    }
    T = prices[0].size();
}

void build_adj(int row, int col) {
    int r = row * Col + col;
    int hx[] = {0, 0, 1, -1};
    int hy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int arow = row + hx[i];
        int acol = col + hy[i];
        if(arow >= 0 && arow < Row && acol >=0 && acol < Col) {
            int next_r = arow * Col + acol;
            adj[r].push_back(next_r);
        }
    }
}

void read_rack_data() {
    Row = 6;
    Col = 3;
    R = Col * Row;
    vector<vector<int>> init_rack_conf(Row,vector<int>(Col, SUNLIGHT));
    rack_conf = init_rack_conf;
    vector<vector<int>> init_adj(R, vector<int>());
    adj = init_adj;
    for(int i = 0; i < Row; ++i) {
        for(int j = 0; j < Col; ++j) {
            build_adj(i, j);
            if(predefined_conf.size() > 0) {
                rack_conf[i][j] = predefined_conf[i * Col + j];
            }
            else {
                if(i % 2 && j % 2) {
                        rack_conf[i][j] = NO_SUNLIGHT;
                }
            }
        }
    }
}

bool compare_cultivation(Cultivation u, Cultivation v) {
    if (u.price != v.price) {
        return u.price < v.price;
    }
    else {
        return cultivate_time[u.crop_id] > cultivate_time[v.crop_id];
    }
}

void read_data() {
    read_crop_data();
    read_price_data();
    read_rack_data();
}

void init() {
    revenue = 0;

    vector <vector<double>> init_U(I, vector<double>(T));
    U = init_U;
    cultivations.clear();
    shortest_cultivation.clear();
    for (int i = 0; i < I; ++i) {
        for(int w = 0; w < T; ++w) {
            if (w + 1 < cultivate_time[i]) {
                continue;
            }
            U[i][w] = prices[i][w] * harvested_amount[i] / cultivate_time[i];
            cultivations.push_back(Cultivation(U[i][w], i, w));
        }
    }

    sort(cultivations.begin(), cultivations.end(), compare_cultivation);

    int shortest;
    for(int i = 0; i < cultivations.size(); ++i) {
        if (i == 0) {
            shortest = cultivate_time[cultivations[0].crop_id];
        }
        else {
            shortest = min(shortest_cultivation.back(), cultivate_time[cultivations[i].crop_id]);
        }
        shortest_cultivation.push_back(shortest);
    }

    vector<int> init_avail_slot(R, T);
    avail_slot = init_avail_slot;
    vector<vector<int>> init_rack_timeslot(R, vector<int>(T, -1));
    rack_timeslot = init_rack_timeslot;
    vector<vector<Cultivation>> init_rack_cultivations(R);
    rack_cultivations = init_rack_cultivations;
}

bool is_rack_available(int r, vector<Cultivation> cultivations, vector<int> shortest_cultivation) {
    if (cultivations.size() == 0) {
        return false;
    }
    if (avail_slot[r] <= shortest_cultivation.back()) {
        return false;
    }
    return true;
}

bool can_insert(Cultivation cultivation, int r) {
    if(cultivation.start_week() < 0) {
        return false; 
    }

    for (int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
        if (rack_timeslot[r][w] != -1) {
            return false;
        }
    }
    return true;
}

bool have_sunlight(int r) {
    int col = r % Col;
    int row = r / Col;
    return rack_conf[row][col];

}

bool is_family(int u, int v) {
    if(u == -1 || v == -1) return false;
    return family_name[u] == family_name[v];
}

bool is_valid (Cultivation cultivation, int r) {
    if (sunlight_requirement[cultivation.crop_id] && !have_sunlight(r)) {
        return false;
    }
    
    if(cultivation.start_week() < 0) {
        return false;
    }

    for(int i = 0; i < adj[r].size(); ++i) {
        int adj_rack = adj[r][i];
        for(int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
            if (is_family(rack_timeslot[adj_rack][w], cultivation.crop_id)) {
                return false;
            }
        }
    }
    return true;
}

void insert_cultivation(Cultivation cultivation, int r) {
    rack_cultivations[r].push_back(cultivation);
    for(int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
        rack_timeslot[r][w] = cultivation.crop_id;
    }
    avail_slot[r] -= cultivate_time[cultivation.crop_id];
    revenue += prices[cultivation.crop_id][cultivation.harvest_week] * harvested_amount[cultivation.crop_id] * 0.95;
}

bool try_cultivation(Cultivation cultivation, int r) {
    if (can_insert(cultivation, r)) {
        if (is_valid(cultivation, r)) {
            insert_cultivation(cultivation, r);
            return true;
        }
    }
    return false;
}

void schedule_rack(int r) {
    vector<Cultivation> clone_cultivations = cultivations;
    vector<int> clone_shortest_cultivation = shortest_cultivation;
    while(is_rack_available(r, clone_cultivations, clone_shortest_cultivation)) {
        Cultivation best_cultivation = clone_cultivations.back();
        if (try_cultivation(best_cultivation, r)) {
            continue;
        }

        Cultivation forward_cultivation = best_cultivation;
        forward_cultivation.harvest_week -= 1;
        if (try_cultivation(forward_cultivation, r)) {
            continue;
        }

        Cultivation delay_cultivation = best_cultivation;
        forward_cultivation.harvest_week += 1;
        if (try_cultivation(forward_cultivation, r)) {
            continue;
        }
        clone_cultivations.pop_back();
        clone_shortest_cultivation.pop_back();
    }
}

void schedule_all() {
    vector<vector<Cultivation>> clone_cultivations(R);
    vector<vector<int>> clone_shortest_cultivation(R);
    for(int r = 0; r < R; ++r) {
        clone_cultivations[r] = cultivations;
        clone_shortest_cultivation[r] = shortest_cultivation;
    }
    bool keep_schedule = true;
    while (keep_schedule) {
        keep_schedule = false;
        for(int r = 0; r < R; ++r) {
            if (is_rack_available(r, clone_cultivations[r], clone_shortest_cultivation[r])) {
                keep_schedule = true; 
                Cultivation best_cultivation = clone_cultivations[r].back();
                if (try_cultivation(best_cultivation, r)) {
                    continue;
                }

                Cultivation forward_cultivation = best_cultivation;
                forward_cultivation.harvest_week -= 1;
                if (try_cultivation(forward_cultivation, r)) {
                    continue;
                }

                Cultivation delay_cultivation = best_cultivation;
                forward_cultivation.harvest_week += 1;
                if (try_cultivation(forward_cultivation, r)) {
                    continue;
                }
                clone_cultivations[r].pop_back();
                clone_shortest_cultivation[r].pop_back();
            }
        }
    }

}

void schedule_rack_with_DP(int r) {
    vector<vector<double>> DP(T, vector<double>(2));
    vector<vector<int>> trace(T, vector<int>(2));

    for(int w = 0; w < T; ++w) {
        DP[w][0] = DP[w][1] = 0;
        trace[w][0] = trace[w][1] = -1;
        if(w > 0) {
            DP[w][0] = DP[w - 1][0];
            DP[w][1] = max(DP[w - 1][w], DP[w - 1][0]);
        }

        for(int i = 0; i < I; ++i) {
            double value = prices[i][w] * harvested_amount[i] * 0.95;
            Cultivation cultivation(value, i, w);
            if(!can_insert(cultivation, r) || !is_valid(cultivation, r)) {
                continue;
            }
            if (cultivate_time[i] <= w + 1) {
                for(int k = 0; k < 2; ++k) {
                    if (cultivate_time[i] == w + 1) {
                        if (k == 0 && value > DP[w][k]) {
                            DP[w][k] = value;
                            trace[w][k] = i;
                        }
                    } else {
                        if (DP[w - cultivate_time[i]][k] + value > DP[w][k]) {
                            DP[w][k] = DP[w - cultivate_time[i]][k] + value;
                            trace[w][k] = i;
                        }
                    }
                }
            }
        }
    }
    
    int w = T - 1;
    int k = 1;
    while(w >= 0) {
        if (trace[w][k] != -1) {
            int i = trace[w][k];
            Cultivation cultivation(0, i, w);
            insert_cultivation(cultivation, r);
            w -= cultivate_time[i];
        }
        else {
            if(k == 1 && DP[w][1] == DP[w - 1][0]) 
                k = 0;
            w -= 1;
        }
    }
}

void write_result() {
    //freopen("c:\\Users\\kurod\\Documents\\CPP\\HPFS\\result.csv", "w", stdout);
    ofstream out("c:\\Users\\kurod\\Documents\\CPP\\HPFS\\result.csv");
    for(int r = 0; r < R; ++r) {
        for(int w = 0; w < T; ++w) {
            if (rack_timeslot[r][w] != -1) {
                out << crop_name[rack_timeslot[r][w]] << ",";
            }
            else {
                out << -1 << ",";
            }
        }
        out << endl;
    }
}

void shuffle_rack() {
auto rng = std::default_random_engine {};
std::shuffle(std::begin(rack_num), std::end(rack_num), rng);
}

void run_HPFS() {
    init();
    for(int r = 0; r < R; ++r) {
        schedule_rack(r);
    }
    write_result();
    cout << revenue << endl;
}

void run_HPFS_DP() {
    init();
    for(int r = 0; r < R; ++r) {
        schedule_rack_with_DP(rack_num[r]);
    }
    if(revenue > best_revenue) {
        best_revenue = revenue;
        write_result();
    }
    cout << revenue << endl;
}

void run_HPFS_schedule_all() {
    init();
    schedule_all();
    write_result();
    cout << revenue << endl;
}

int main() {
    read_data();
    vector<int> init_rack_num(R);
    rack_num = init_rack_num;
    for(int i = 0; i < R; ++i) rack_num[i] = i;
    cout << rack_num.size() << endl;
    for(int i = 0; i < R; ++i) {
        shuffle_rack();
        run_HPFS_DP();
    }
    cout << best_revenue << endl;
}
