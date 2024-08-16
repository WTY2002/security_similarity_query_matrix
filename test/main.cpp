#include <Matrix_encryption.h>
#include <SSQ.h>

/**
 * @Description: 基于矩阵加密的内积符号判定测试
 */
void test() {
    int N = 3;  // 矩阵的大小
    vector<double> a = {1, -2, -3};
    vector<double> b = {4, 5, 6};

    VectorXd v1 = Eigen::Map<VectorXd>(a.data(), a.size());
    VectorXd v2 = Eigen::Map<VectorXd>(b.data(), b.size());

    cout << v1.dot(v2) << endl;


    cout << endl << "=====================" << endl;


    // 生成一个可逆矩阵
    MatrixXd matrix = generateInvertibleMatrix(N);
    cout << "加密矩阵:\n" << matrix << endl;

    cout << endl << "=====================" << endl;

    // // 计算矩阵 * 向量 v1
    // VectorXd t1 = matrix * v1;
    // cout << "Matrix * v1:\n" << t1 << endl;

    // 计算 v1 * matrix，v1 作为行向量
    VectorXd t1 = matrix.transpose() * v1;
    cout << "v1 * matrix (as row vector):\n" << t1.transpose() << endl;

    cout << endl << "=====================" << endl;

    // 解密还原v1，即计算逆矩阵 * t1
    VectorXd v1_decrypted = t1.transpose() * calculateInverseMatrix(matrix);
    cout << "解密还原后的向量 v1:\n" << v1_decrypted << endl;

    cout << endl << "=====================" << endl;



    // 生成该矩阵的逆矩阵
    MatrixXd inverseMatrix = calculateInverseMatrix(matrix);
    cout << "逆矩阵:\n" << inverseMatrix << endl;

    cout << endl << "=====================" << endl;

    // 计算逆矩阵 * 向量 v2
    VectorXd t2 = inverseMatrix * v2;
    cout << "InverseMatrix * v2:\n" << t2 << endl;

    cout << endl << "=====================" << endl;

    // 计算 t1 和 t2 的点积
    double t3 = t1.dot(t2);
    cout << "Dot product of t1 and t2:\n" << t3 << endl;

    cout << endl << "=====================" << endl;

    // 计算 matrix * inverseMatrix
    MatrixXd result = matrix * inverseMatrix;
    cout << "加密矩阵 * 逆矩阵:\n" << result << endl;

    cout << endl << "=====================" << endl;
}

// 自定义比较器，仅根据 double 值排序
struct Compare {
    bool operator()(const pair<double, VectorXd>& a, const pair<double, VectorXd>& b) {
        return a.first < b.first;  // 使 priority_queue 按降序排序
    }
};

int main() {

    // test();

    // 生成一个可逆矩阵
    // MatrixXd matrix = generateInvertibleMatrix(5);
    //
    // MatrixXd inverseMatrix = calculateInverseMatrix(matrix);
    //
    // cout << "矩阵:\n" << matrix << endl;
    // cout << "--------------------------------------------" << endl;
    // cout << "逆矩阵:\n" << inverseMatrix << endl;
    // cout << "--------------------------------------------" << endl;
    // cout << "逆矩阵的逆矩阵:\n" << calculateInverseMatrix(inverseMatrix) << endl;

    // vector<double> a = {1, -2, -3};
    // vector<double> b = {4, 5, 6};
    //
    // VectorXd v1 = Eigen::Map<VectorXd>(a.data(), a.size());
    // VectorXd v2 = Eigen::Map<VectorXd>(b.data(), b.size());
    //
    //
    // // 使用自定义比较器的 priority_queue
    // priority_queue<pair<double, VectorXd>, vector<pair<double, VectorXd>>, Compare> heap;
    // heap.push({3, v1});
    // heap.push({5, v1});
    // heap.push({1, v1});
    // heap.push({2, v1});
    // heap.push({6, v1});
    //
    // while (!heap.empty()) {
    //     cout << heap.top().first << " " << heap.top().second.transpose() << endl;
    //     heap.pop();
    // }

    char* fileString = "/root/wty/data.txt";
    char* query = "/root/wty/query.txt";
    char* res = "/root/wty/result.txt";


    auto start_time = chrono::high_resolution_clock::now();

    dealData(fileString); // 预处理数据

    // 获取结束时间点
    auto end_time = chrono::high_resolution_clock::now();
    // 计算时间间隔
    chrono::duration<double, milli> total_duration = end_time - start_time;
    // 输出时间间隔
    printf("数据加密外包的时间是：%f 毫秒\n", total_duration.count());
    fflush(stdout);

    auto start_time2 = chrono::high_resolution_clock::now();

    SSQ(query, res); // 查询

    // 获取结束时间点
    auto end_time2 = chrono::high_resolution_clock::now();
    // 计算时间间隔
    chrono::duration<double, milli> total_duration2 = end_time2 - start_time2;
    // 输出时间间隔
    printf("查询的总时间是：%f 毫秒\n", total_duration2.count());
    fflush(stdout);



    return 0;
}
