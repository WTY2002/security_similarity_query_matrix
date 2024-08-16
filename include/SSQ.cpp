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


/**
 * @Method: readDataFromFile
 * @Description: 读取文件中的doubles，并返回一个vector<vector<double>>类型的数据
 * @param char* filename 文件名
 * @return vector<vector<double>> doubles数据
 */
vector<vector<double>> readDataFromFile(char* filename) {
    vector<vector<double>> data_list;
    ifstream infile(filename);

    if (!infile.is_open()) {
        cerr << "Error opening file" << endl;
        return data_list;
    }

    string line;
    while (getline(infile, line)) {
        vector<double> row;
        stringstream ss(line);
        double number;

        while (ss >> number) {
            row.push_back(number);
        }

        data_list.push_back(row);
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
 * @Method: 读取数据集
 * @param char* fileString 读取数据集的地址
 * @return 状态码，1：成功；0：失败
 */
int dealData(char* fileString) {
    auto start_time = chrono::high_resolution_clock::now();
    // 读取数据
    vector<vector<double>> data_list = readDataFromFile(fileString);

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

    // 对每一个明文数据进行加密
    for (int i = 0; i < data_list.size(); i++) {
        vector<double> t(data_list[i].size() + 3);
        // 计算每一维数据的平方和
        double quadratic_sum = 0;
        for (int j = 0; j < data_list[i].size(); j++) {
            quadratic_sum += data_list[i][j] * data_list[i][j];
            t[j + 1] = data_list[i][j] * -2;
        }
        t[0] = quadratic_sum;

        // 生成一个随机数r11，确保r11 > 0
        double r11 = generateRandomDouble();
        t[data_list[i].size() + 1] = r11;
        t[data_list[i].size() + 2] = -r11;

        VectorXd v = Eigen::Map<VectorXd>(t.data(), t.size()); // 将vector<double>转换为Eigen::VectorXd
        // 加密
        v = encryptMatrix.transpose() * v;

        ciphertext[i] = v;
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

    priority_queue<pair<double, VectorXd>, vector<pair<double, VectorXd>>, Compare> heap; // 优先队列，用于存储查询结果

    double distance; // 欧式平方距离

    for (int i = 0; i < ciphertext.size(); i++) {
        distance = ciphertext[i].dot(q);
        if (heap.size() < query_data[0][0]) { // 维护大小为k的优先队列
            heap.push(make_pair(distance, ciphertext[i]));
        } else {
            if (heap.top().first > distance) {
                heap.pop();
                heap.push(make_pair(distance, ciphertext[i]));
            }
        }
    }

    // 将heap内的数据写入文件
    ofstream resultFile(resultFilePath);
    if (resultFile.is_open()) {
        while (!heap.empty()) {
            // 将ciphertext[i]解密
            VectorXd decryptedVector = heap.top().second.transpose() * encryptMatrixInverse;
            for (int j = 1; j < decryptedVector.size() - 2; j++) {
                resultFile << decryptedVector[j] / (-2) << " "; // 写入文件
            }
            resultFile << endl; // 换行
            heap.pop(); // 弹出堆顶元素
        }
        resultFile.close(); // 关闭文件
    } else {
        cerr << "Unable to open file " << resultFilePath << endl;
        return 0;
    }
    return 1;
}