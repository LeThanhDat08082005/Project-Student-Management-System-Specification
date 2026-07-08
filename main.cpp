1. Quản lý chương trình học
// Lay danh sach chuong trinh hoc 
svr.Get("/api/curriculums", [](const httplib::Request&, httplib::Response& res) {
        sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt; sqlite3_prepare_v2(db, "SELECT curriculum_id, major, academic_intake, total_credits FROM CURRICULUMS;", -1, &stmt, 0); std::string json = "["; bool first = true;
        while (sqlite3_step(stmt) == SQLITE_ROW) { if (!first) json += ","; first = false; json += "{\"id\":\"" + safe_column_text(stmt, 0) + "\",\"major\":\"" + safe_column_text(stmt, 1) + "\",\"intake\":\"" + safe_column_text(stmt, 2) + "\",\"total_credits\":" + std::to_string(sqlite3_column_int(stmt, 3)) + "}"; }
        json += "]"; sqlite3_finalize(stmt); sqlite3_close(db); res.set_content(json, "application/json; charset=utf-8");
    });

// Them chuong trinh hoc
svr.Post("/api/curriculums", [](const httplib::Request& req, httplib::Response& res) {
        std::string id = get_json_value(req.body, "curriculum_id"), major = get_json_value(req.body, "major"), intake = get_json_value(req.body, "academic_intake"), credits = get_json_value(req.body, "total_credits");
        sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "INSERT INTO CURRICULUMS (curriculum_id, major, academic_intake, total_credits) VALUES (?, ?, ?, ?);", -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_text(stmt, 2, major.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_text(stmt, 3, intake.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 4, std::stoi(credits));
        sqlite3_step(stmt); sqlite3_finalize(stmt); sqlite3_close(db); res.status = 201; res.set_content("{\"status\":\"success\"}", "application/json");
    });
    

2. Quản lý môn học 
svr.Post("/api/courses", [](const httplib::Request& req, httplib::Response& res) {
        std::string code = get_json_value(req.body, "code"), name = get_json_value(req.body, "name"), credits = get_json_value(req.body, "credits"); sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "INSERT INTO COURSES (course_code, course_name, credits, semester, is_elective) VALUES (?, ?, ?, 1, 0);", -1, &stmt, 0); sqlite3_bind_text(stmt, 1, code.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 3, std::stoi(credits)); sqlite3_step(stmt); sqlite3_finalize(stmt); sqlite3_close(db); res.status = 201; res.set_content("{\"status\":\"success\"}", "application/json");
    });


3 & 4. Quan ly lop hoc phan va ghi danh 
// Them lop hoc phan 
svr.Post("/api/classes", [](const httplib::Request& req, httplib::Response& res) {
    std::string class_code = get_json_value(req.body, "class_code");
    std::string course_id = get_json_value(req.body, "course_id");
    std::string lecturer_id = get_json_value(req.body, "lecturer_id");
    
});

// Ghi danh sinh vien vao lop 

svr.Post("/api/enrollments", [](const httplib::Request& req, httplib::Response& res) {
    std::string student_id = get_json_value(req.body, "student_id");
    std::string class_id = get_json_value(req.body, "class_id");

    sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO ENROLLMENTS (student_id, class_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, std::stoi(student_id));
        sqlite3_bind_int(stmt, 2, std::stoi(class_id));
        sqlite3_step(stmt);
        res.status = 201; res.set_content("{\"status\":\"success\"}", "application/json");
    }
    sqlite3_finalize(stmt); sqlite3_close(db);
});

5 & 6. Quản lý và xem điểm 
// Xem điểm (Lọc theo class_id hoặc student_id)
svr.Get("/api/grades", [](const httplib::Request& req, httplib::Response& res) {
    std::string class_id = req.get_param_value("class_id");
    sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt;
    
    std::string sql = "SELECT E.enroll_id, S.student_code, S.full_name, E.process_grade, E.mid_grade, E.final_grade "
                      "FROM ENROLLMENTS E JOIN STUDENTS S ON E.student_id = S.student_id WHERE E.class_id = " + class_id;
    
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    std::string json = "["; bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) json += ","; first = false;
        json += "{\"enroll_id\":" + std::to_string(sqlite3_column_int(stmt, 0)) + ","
                "\"student_code\":\"" + safe_column_text(stmt, 1) + "\","
                "\"student_name\":\"" + safe_column_text(stmt, 2) + "\","
                "\"process_grade\":" + std::to_string(sqlite3_column_double(stmt, 3)) + ","
                "\"mid_grade\":" + std::to_string(sqlite3_column_double(stmt, 4)) + ","
                "\"final_grade\":" + std::to_string(sqlite3_column_double(stmt, 5)) + "}";
    }
    json += "]"; sqlite3_finalize(stmt); sqlite3_close(db);
    res.set_content(json, "application/json; charset=utf-8");
});
 
// Cập nhật điểm
svr.Post("/api/grades/update", [](const httplib::Request& req, httplib::Response& res) {
    
});

7.Báo cáo thống kê
svr.Get("/api/stats/summary", [](const httplib::Request&, httplib::Response& res) {
    sqlite3* db; sqlite3_open("student_system.db", &db); sqlite3_stmt* stmt;
    int total_students = 0, total_lecturers = 0;
    
    // Đếm tổng số sinh viên
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM STUDENTS;", -1, &stmt, 0);
    if (sqlite3_step(stmt) == SQLITE_ROW) total_students = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
 
    // Đếm tổng số giảng viên
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM LECTURERS;", -1, &stmt, 0);
    if (sqlite3_step(stmt) == SQLITE_ROW) total_lecturers = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    
    sqlite3_close(db);

    std::string json = "{"
                       "\"total_students\":" + std::to_string(total_students) + ","
                       "\"total_lecturers\":" + std::to_string(total_lecturers) + 
                       "}";
    res.set_content(json, "application/json; charset=utf-8");
});
