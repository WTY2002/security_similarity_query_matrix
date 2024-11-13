/**
* @author: WTY
* @date: 2024/8/15
* @description: Definition of constants, operations, and header files
*/

#ifndef SSQ_H
#define SSQ_H

#include "Matrix_encryption.h"
#include<queue>
#include <fstream>
#include <string>

#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/**
	 * @Method: readDataFromFile
	 * @Description: 读取文件中的doubles，并返回一个vector<vector<double>>类型的数据
	 * @param char* filename 文件名
	 * @return vector<vector<double>> doubles数据
	 */
	vector<vector<double>> readDataFromFile1(char* filename);

	/**
	 * @Method: readDataFromFile
	 * @Description: 读取文件中指定行的doubles，并返回一个vector<double>类型的数据
	 * @param const char* filename 文件名
	 * @param int lineNumber 行号
	 * @return vector<double> doubles数据
	 */
	vector<double> readDataFromFile2(const char* filename, int lineNumber);

	/**
	 * @Method: 读取数据集
	 * @param char* fileString 读取数据集的地址
	 * @return 状态码，1：成功；0：失败
	 */
	int dealData(char* fileString);

	/**
	 * @Method: SSQ
	 * @Description: 发起查询请求，并返回查询结果
	 * @param char* fileString 读取数据的地址
	 * @param char* resultFilePath 输出数据的地址
	 * @return 状态码，1：成功；0：失败
	 */
	int SSQ(char* fileString, char* resultFilePath);

	EXPORT_SYMBOL int init_algo(char* fileString);
	EXPORT_SYMBOL int query_algo(char* fileString, char* resultFilePath);


#ifdef __cplusplus
}
#endif

#endif //SSQ_H
