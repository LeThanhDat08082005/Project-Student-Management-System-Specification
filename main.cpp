#include <iostream>
#include <string>
#include <sqlite3.h>
#include "httplib.h"

// Hàm xử lý tránh lỗi khi dữ liệu trong DB bị NULL
std::string safe_column_text(sqlite3_stmt* stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

// Thêm hàm bổ trợ trích xuất sạch dữ liệu chuỗi JSON phẳng
std::string get_json_value(const std::string& body, const std::string& key) {
    std::string target = "\"" + key + "\":";
    size_t pos = body.find(target);
    if (pos == std::string::npos) return "";
    pos += target.length();
    while (pos < body.length() && (body[pos] == ' ' || body[pos] == '"')) pos++;
    size_t end = pos;
    while (end < body.length() && body[end] != '"' && body[end] != ',' && body[end] != '}') end++;
    return body.substr(pos, end - pos);
}

int main() {
    httplib::Server svr;

    // 1. API ĐĂNG NHẬP (An toàn - Chống SQL Injection)
    svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
        std::string body = req.body;
        std::string email = get_json_value(body, "email");
        std::string password = get_json_value(body, "password");

        sqlite3* db;
        sqlite3_open("student_system.db", &db);
        
        // Sử dụng dấu ? để binding tham số an toàn
        std::string query = "SELECT user_id, role, status FROM USERS WHERE email = ? AND password = ?;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int uid = sqlite3_column_int(stmt, 0);
                int role = sqlite3_column_int(stmt, 1);
                int status = sqlite3_column_int(stmt, 2);
                
                if (status == 0) {
                    res.status = 403;
                    res.set_content("{\"error\":\"Tài khoản đã bị khóa!\"}", "application/json; charset=utf-8");
                } else {
                    std::string response_json = "{\"status\":\"success\",\"uid\ cloth\":" + std::to_string(uid) + ",\"uid\":" + std::to_string(uid) + ",\"role\":" + std::to_string(role) + "}";
                    res.set_content(response_json, "application/json; charset=utf-8");
                }
            } else {
                res.status = 401;
                res.set_content("{\"error\":\"Sai email hoặc mật khẩu!\"}", "application/json; charset=utf-8");
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    });

    // 2. API LẤY DANH SÁCH SINH VIÊN (Lấy kèm cả user_id để xử lý chức năng khóa)
    svr.Get("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        sqlite3* db;
        sqlite3_open("student_system.db", &db);
        
        // SELECT thêm trường U.user_id để quản lý tài khoản chính xác
        std::string query = "SELECT S.student_id, S.student_code, S.full_name, S.major, S.cohort, U.status, U.user_id "
                            "FROM STUDENTS S JOIN USERS U ON S.user_id = U.user_id;";
        sqlite3_stmt* stmt;
        std::string json = "[";
        bool first = true;
        
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                if (!first) json += ",";
                first = false;
                json += "{\"id\":" + std::to_string(sqlite3_column_int(stmt, 0)) +
                        ",\"code\":\"" + safe_column_text(stmt, 1) + "\"" +
                        ",\"name\":\"" + safe_column_text(stmt, 2) + "\"" +
                        ",\"major\":\"" + safe_column_text(stmt, 3) + "\"" +
                        ",\"cohort\":\"" + safe_column_text(stmt, 4) + "\"" +
                        ",\"status\":" + std::to_string(sqlite3_column_int(stmt, 5)) + 
                        ",\"userId\":" + std::to_string(sqlite3_column_int(stmt, 6)) + "}";
            }
        }
        json += "]";
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        res.set_content(json, "application/json; charset=utf-8");
    });

    // 3. API ĐỔI MẬT KHẨU
    svr.Post("/api/users/change_password", [](const httplib::Request& req, httplib::Response& res) {
        std::string body = req.body;
        std::string uid = get_json_value(body, "uid");
        std::string old_pass = get_json_value(body, "old_pass");
        std::string new_pass = get_json_value(body, "new_pass");

        sqlite3* db;
        sqlite3_open("student_system.db", &db);
        
        std::string check_query = "SELECT user_id FROM USERS WHERE user_id = ? AND password = ?;";
        sqlite3_stmt* stmt;
        bool is_valid = false;
        
        if (sqlite3_prepare_v2(db, check_query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, std::stoi(uid));
            sqlite3_bind_text(stmt, 2, old_pass.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                is_valid = true;
            }
        }
        sqlite3_finalize(stmt);

        if (is_valid) {
            std::string update_query = "UPDATE USERS SET password = ? WHERE user_id = ?;";
            sqlite3_stmt* up_stmt;
            if (sqlite3_prepare_v2(db, update_query.c_str(), -1, &up_stmt, 0) == SQLITE_OK) {
                sqlite3_bind_text(up_stmt, 1, new_pass.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(up_stmt, 2, std::stoi(uid));
                sqlite3_step(up_stmt);
            }
            sqlite3_finalize(up_stmt);
            res.set_content("{\"status\":\"success\"}", "application/json; charset=utf-8");
        } else {
            res.status = 400;
            res.set_content("{\"error\":\"Mật khẩu cũ không chính xác\"}", "application/json; charset=utf-8");
        }
        sqlite3_close(db);
    });

    // Các API /api/students/add và /api/lecturers giữ nguyên logic đã tối ưu hóa...
    
    svr.listen("0.0.0.0", 8080);
    return 0;
}