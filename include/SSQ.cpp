/**
* @author: WTY
* @date: 2024/8/15
* @description: Definition of constants, operations, and header files
*/

#include "SSQ.h"

// 密文数据集
vector<VectorXd> ciphertext;

// 加密矩阵
MatrixXd encryptMatrix;

// 自定义比较器，仅根据 double 值排序
struct Compare {
    bool operator()(const pair<double, VectorXd>& a, const pair<double, VectorXd>& b) {
        return a.first < b.first;  // 使 priority_queue 按降序排序
    }
};


// 全局变量,用于保护共享资源global_heap
mutex global_heap_mutex;
// 优先队列，用于存储最近k个查询结果
priority_queue<pair<double, VectorXd>, vector<pair<double, VectorXd>>, Compare> global_heap;


/**
 * @Method: readDataFromFile
 * @Description: 读取文件中的doubles，并返回一个vector<vector<double>>类型的数据
 * @param char* filename 文件名
 * @return vector<vector<double>> doubles数据
 */
// vector<vector<double>> readDataFromFile(const char* filename) {
//     vector<vector<double>> data_list;
//     ifstream infile(filename);
//
//     if (!infile.is_open()) {
//         cerr << "Error opening file" << endl;
//         return data_list;
//     }
//
//     string line;
//     while (getline(infile, line)) {
//         vector<double> row;
//         stringstream ss(line);
//         double number;
//
//         while (ss >> number) {
//             row.push_back(number);
//         }
//
//         data_list.push_back(row);
//     }
//
//     infile.close();
//     return data_list;
// }

/**
 * @Method: readChunk
 * @Description: 读取文件中的doubles，并返回一个vector<vector<double>>类型的数据
 * @param const char* filename 文件名
 * @param vector<vector<double>>& thread_data doubles数据
 * @param size_t start 起始位置
 * @param size_t end 结束位置
 */
void readChunk(const char* filename, vector<vector<double>>& thread_data, size_t start, size_t end) {
    ifstream infile(filename);
    infile.seekg(start);

    // 如果不是文件开头，则丢弃第一行，避免读取不完整的行
    if (start != 0) {
        string dummy;
        getline(infile, dummy);
    }

    string line;
    size_t position = infile.tellg();
    while (position < end && getline(infile, line)) {
        vector<double> row;
        stringstream ss(line);
        double number;
        while (ss >> number) {
            row.push_back(number);
        }

        thread_data.push_back(row);
        position = infile.tellg(); // 更新位置
    }
}

/**
 * @Method: readDataFromFile
 * @Description: 读取文件中的doubles，并返回一个vector<vector<double>>类型的数据
 * @param char* filename 文件名
 * @return vector<vector<double>> doubles数据
 */
vector<vector<double>> readDataFromFile(const char* filename) {
    vector<vector<double>> data_list;
    ifstream infile(filename, ios::binary | ios::ate);

    if (!infile.is_open()) {
        cerr << "Error opening file" << endl;
        return data_list;
    }

    size_t file_size = infile.tellg();
    infile.seekg(0);

    // size_t num_threads = thread::hardware_concurrency(); // 获取系统支持的线程数
    size_t num_threads = 30; // 获取系统支持的线程数
    size_t chunk_size = file_size / num_threads;

    vector<thread> threads;
    vector<vector<vector<double>>> thread_data(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;

        threads.emplace_back(readChunk, filename, ref(thread_data[i]), start, end);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 合并所有线程的数据到data_list中
    for (auto& partial_data : thread_data) {
        data_list.insert(data_list.end(), partial_data.begin(), partial_data.end());
    }

    infile.close();
    return data_list;
}

/**
 * @Method: readDataFromFile
 * @Description: 读取文件中指定行的doubles，并返回一个vector<double>类型的数据
 * @param const char* filename 文件名
 * @param int lineNumber 行号
 * @return vector<double> doubles数据
 */
vector<double> readDataFromFile(const char* filename, int lineNumber) {
    ifstream infile(filename);
    string line;
    vector<double> result;
    int currentLine = 0;

    while (getline(infile, line)) {
        if (++currentLine == lineNumber) {
            istringstream iss(line);
            double number;
            while (iss >> number) {
                result.push_back(number);
            }
            break;
        }
    }
    return result;
}

/**
 * @Method: encryptDataChunk
 * @Description: 对数据块进行加密
 * @param int start 数据块起始位置
 * @param int end 数据块结束位置
 * @param vector<vector<double>>& data_list 数据块
 */
void encryptDataChunk(int start, int end, vector<vector<double>>& data_list) {
    for (int i = start; i < end; i++) {
        vector<double> t(data_list[i].size() + 3);
        double quadratic_sum = 0;
        for (int j = 0; j < data_list[i].size(); j++) {
            quadratic_sum += data_list[i][j] * data_list[i][j];
            t[j + 1] = data_list[i][j] * -2;
        }
        t[0] = quadratic_sum;

        double r11 = generateRandomDouble();
        t[data_list[i].size() + 1] = r11;
        t[data_list[i].size() + 2] = -r11;

        VectorXd v = Eigen::Map<VectorXd>(t.data(), t.size());
        v = encryptMatrix.transpose() * v;

        ciphertext[i] = v;
    }
}

/**
 * @Method: dealData
 * @Description: 处理数据，包括读取数据、生成加密矩阵、加密数据
 * @param char* fileString 读取数据集的地址
 * @return 状态码，1：成功；0：失败
 */
int dealData(char* fileString) {
    auto start_time = chrono::high_resolution_clock::now();
    // 读取数据
    vector<vector<double>> data_list = readDataFromFile(fileString);

    //
    // // 将data_list中的数据写入文件
    // char* filename = "/root/wty/ttt.txt";
    // ofstream outfile(filename);
    // if (!outfile.is_open()) {
    //     cerr << "Error opening file for writing: " << filename << endl;
    //     return false;
    // }
    //
    // for (const auto& row : data_list) {
    //     for (size_t j = 0; j < row.size(); ++j) {
    //         outfile << row[j];
    //         if (j < row.size() - 1) {
    //             outfile << " "; // Add space between numbers
    //         }
    //     }
    //     outfile << endl; // Newline at the end of each row
    // }
    //
    // outfile.close();



    // 获取结束时间点
    auto end_time = chrono::high_resolution_clock::now();
    // 计算时间间隔
    chrono::duration<double, milli> total_duration = end_time - start_time;
    // 输出时间间隔
    printf("数据读取的时间是：%f 毫秒\n", total_duration.count());
    fflush(stdout);

    if (data_list.empty()) {
        return 0;
    }

    start_time = chrono::high_resolution_clock::now();

    // 生成加密矩阵
    encryptMatrix = generateInvertibleMatrix(data_list[0].size() + 3);

    end_time = chrono::high_resolution_clock::now();
    total_duration = end_time - start_time;
    // 输出时间间隔
    printf("生成加密矩阵的时间是：%f 毫秒\n", total_duration.count());
    fflush(stdout);

    start_time = chrono::high_resolution_clock::now();

    ciphertext.resize(data_list.size()); // 初始化密文数据集的大小

    // // 对每一个明文数据进行加密
    // for (int i = 0; i < data_list.size(); i++) {
    //     vector<double> t(data_list[i].size() + 3);
    //     // 计算每一维数据的平方和
    //     double quadratic_sum = 0;
    //     for (int j = 0; j < data_list[i].size(); j++) {
    //         quadratic_sum += data_list[i][j] * data_list[i][j];
    //         t[j + 1] = data_list[i][j] * -2;
    //     }
    //     t[0] = quadratic_sum;
    //
    //     // 生成一个随机数r11，确保r11 > 0
    //     double r11 = generateRandomDouble();
    //     t[data_list[i].size() + 1] = r11;
    //     t[data_list[i].size() + 2] = -r11;
    //
    //     VectorXd v = Eigen::Map<VectorXd>(t.data(), t.size()); // 将vector<double>转换为Eigen::VectorXd
    //     // 加密
    //     v = encryptMatrix.transpose() * v;
    //
    //     ciphertext[i] = v;
    // }


    // size_t num_threads = thread::hardware_concurrency(); // 获取系统支持的并发线程数
    size_t num_threads = 30; // 获取系统支持的线程数
    size_t chunk_size = data_list.size() / num_threads;

    vector<thread> threads;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? data_list.size() : (i + 1) * chunk_size;

        threads.emplace_back(encryptDataChunk, start, end, ref(data_list));
    }

    for (auto& th : threads) {
        th.join();
    }


    end_time = chrono::high_resolution_clock::now();
    total_duration = end_time - start_time;
    // 输出时间间隔
    printf("加密数据的总时间是：%f 毫秒\n", total_duration.count());
    fflush(stdout);
    cout << "--------------------------------------------" << endl;

    return 1;
}

/**
 * @Method: processChunk
 * @Description: 处理数据块并更新全局优先队列
 * @param const vector<VectorXd>& ciphertext 密文数据集
 * @param const VectorXd& q 查询向量
 * @param size_t start 数据块的起始位置
 * @param size_t end 数据块的结束位置
 * @param size_t k 优先队列的大小
 */
void processChunk(const vector<VectorXd>& ciphertext, const VectorXd& q, size_t start, size_t end, size_t k) {
    priority_queue<pair<double, VectorXd>, vector<pair<double, VectorXd>>, Compare> local_heap;

    for (size_t i = start; i < end; ++i) {
        double distance = ciphertext[i].dot(q);
        if (local_heap.size() < k) {
            local_heap.push(make_pair(distance, ciphertext[i]));
        } else if (local_heap.top().first > distance) {
            local_heap.pop();
            local_heap.push(make_pair(distance, ciphertext[i]));
        }
    }

    // 更新全局优先队列
    lock_guard<mutex> lock(global_heap_mutex);
    while (!local_heap.empty()) {
        if (global_heap.size() < k) {
            global_heap.push(local_heap.top());
        } else if (global_heap.top().first > local_heap.top().first) {
            global_heap.pop();
            global_heap.push(local_heap.top());
        }
        local_heap.pop();
    }
}

/**
 * @Method: SSQ
 * @Description: 发起查询请求，并返回查询结果
 * @param char* fileString 读取数据的地址
 * @param char* resultFilePath 输出数据的地址
 * @return 状态码，1：成功；0：失败
 */
int SSQ(char* fileString, char* resultFilePath) {
    vector<vector<double>> query_data(2); // 读取查询数据
    query_data[0] = readDataFromFile(fileString, 1);
    query_data[1] = readDataFromFile(fileString, 2);

    // 逆矩阵，用于解密
    MatrixXd encryptMatrixInverse = calculateInverseMatrix(encryptMatrix);

    // 将查询数据加密
    vector<double> t(query_data[1].size() + 3);

    // 生成两个随机数r21,r22，确保r21 > 0
    double r21 = generateRandomDouble();
    double r22 = generateRandomDouble();

    for (int i = 0; i < query_data[1].size(); i++) {
        t[i + 1] = query_data[1][i] * r21;
    }
    t[0] = r21;

    t[query_data[1].size() + 1] = r21 * r22;
    t[query_data[1].size() + 2] = r21 * r22;
    VectorXd q = Eigen::Map<VectorXd>(t.data(), t.size());
    q = encryptMatrixInverse * q; // 将q用逆矩阵进行加密

    // priority_queue<pair<double, VectorXd>, vector<pair<double, VectorXd>>, Compare> heap; // 优先队列，用于存储查询结果
    //
    // double distance; // 欧式平方距离
    //
    // for (int i = 0; i < ciphertext.size(); i++) {
    //     distance = ciphertext[i].dot(q);
    //     if (heap.size() < query_data[0][0]) { // 维护大小为k的优先队列
    //         heap.push(make_pair(distance, ciphertext[i]));
    //     } else {
    //         if (heap.top().first > distance) {
    //             heap.pop();
    //             heap.push(make_pair(distance, ciphertext[i]));
    //         }
    //     }
    // }
    //
    // // 将heap内的数据写入文件
    // ofstream resultFile(resultFilePath);
    // if (resultFile.is_open()) {
    //     while (!heap.empty()) {
    //         // 将ciphertext[i]解密
    //         VectorXd decryptedVector = heap.top().second.transpose() * encryptMatrixInverse;
    //         for (int j = 1; j < decryptedVector.size() - 2; j++) {
    //             resultFile << decryptedVector[j] / (-2) << " "; // 写入文件
    //         }
    //         resultFile << endl; // 换行
    //         heap.pop(); // 弹出堆顶元素
    //     }
    //     resultFile.close(); // 关闭文件
    // } else {
    //     cerr << "Unable to open file " << resultFilePath << endl;
    //     return 0;
    // }
    // return 1;

    size_t num_threads = 20; // 设定线程数
    size_t chunk_size = (ciphertext.size() + num_threads - 1) / num_threads;
    vector<thread> threads;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = min(start + chunk_size, ciphertext.size());
        threads.emplace_back(processChunk, cref(ciphertext), cref(q), start, end, query_data[0][0]);
    }

    for (auto& th : threads) {
        th.join();
    }

    // 将heap内的数据写入文件
    ofstream resultFile(resultFilePath);
    if (resultFile.is_open()) {
        while (!global_heap.empty()) {
            // 将ciphertext[i]解密
            VectorXd decryptedVector = global_heap.top().second.transpose() * encryptMatrixInverse;
            for (int j = 1; j < decryptedVector.size() - 2; j++) {
                resultFile << decryptedVector[j] / (-2) << " "; // 写入文件
            }
            resultFile << endl; // 换行
            global_heap.pop(); // 弹出堆顶元素
        }
        resultFile.close(); // 关闭文件
    } else {
        cerr << "Unable to open file " << resultFilePath << endl;
        return 0;
    }
    return 1;
}