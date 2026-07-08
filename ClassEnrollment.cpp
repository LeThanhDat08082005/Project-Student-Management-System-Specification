#include "class_enrollment.h"
#include "sqlite3.h"
#include <string>


std::string safe_column_text_class(sqlite3_stmt* stmt, int col)
{
    const char* text = (const char*)sqlite3_column_text(stmt, col);
    return text ? std::string(text) : "";
}


std::string get_json_value_class(
    const std::string& json,
    const std::string& key
)
{
    size_t pos = json.find("\"" + key + "\"");

    if(pos == std::string::npos)
        return "";


    size_t start = json.find(":", pos);

    size_t quote_start = json.find("\"", start);


    if(quote_start != std::string::npos &&
       quote_start < json.find_first_of(",}", start))
    {
        size_t quote_end = json.find("\"", quote_start + 1);

        return json.substr(
            quote_start + 1,
            quote_end - quote_start - 1
        );
    }


    size_t num_start = json.find_first_not_of(" :", start);

    size_t num_end = json.find_first_of(",}", num_start);


    return json.substr(
        num_start,
        num_end - num_start
    );
}



void class_enrollment_routes(httplib::Server& svr)
{


// =========================
// QUẢN LÝ LỚP HỌC PHẦN
// =========================


// Thêm lớp học phần
svr.Post(
"/api/classes",
[](const httplib::Request& req,
httplib::Response& res)
{

    std::string code =
    get_json_value_class(req.body,"code");


    std::string course_id =
    get_json_value_class(req.body,"course_id");


    std::string lecturer_id =
    get_json_value_class(req.body,"lecturer_id");


    std::string day =
    get_json_value_class(req.body,"schedule_day");


    std::string time =
    get_json_value_class(req.body,"schedule_time");


    std::string room =
    get_json_value_class(req.body,"room");



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "INSERT INTO COURSE_CLASSES "
        "(class_code, course_id, lecturer_id, schedule_day, schedule_time, room)"
        " VALUES (?, ?, ?, ?, ?, ?);",
        -1,
        &stmt,
        0
    );



    sqlite3_bind_text(
        stmt,
        1,
        code.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_int(
        stmt,
        2,
        std::stoi(course_id)
    );


    sqlite3_bind_int(
        stmt,
        3,
        std::stoi(lecturer_id)
    );


    sqlite3_bind_text(
        stmt,
        4,
        day.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_text(
        stmt,
        5,
        time.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_text(
        stmt,
        6,
        room.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_step(stmt);



    sqlite3_finalize(stmt);

    sqlite3_close(db);



    res.status = 201;

    res.set_content(
        "{\"status\":\"success\"}",
        "application/json"
    );


});




// =========================
// GHI DANH SINH VIÊN
// =========================


svr.Post(
"/api/enrollments",
[](const httplib::Request& req,
httplib::Response& res)
{


    std::string student_id =
    get_json_value_class(
        req.body,
        "student_id"
    );


    std::string class_id =
    get_json_value_class(
        req.body,
        "class_id"
    );



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "INSERT INTO ENROLLMENTS "
        "(student_id,class_id)"
        " VALUES (?,?);",
        -1,
        &stmt,
        0
    );



    sqlite3_bind_int(
        stmt,
        1,
        std::stoi(student_id)
    );


    sqlite3_bind_int(
        stmt,
        2,
        std::stoi(class_id)
    );



    sqlite3_step(stmt);



    sqlite3_finalize(stmt);

    sqlite3_close(db);



    res.status = 201;


    res.set_content(
        "{\"status\":\"success\"}",
        "application/json"
    );


});



}