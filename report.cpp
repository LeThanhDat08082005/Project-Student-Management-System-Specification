#include "report.h"
#include "sqlite3.h"
#include <string>


void report_routes(httplib::Server& svr)
{


svr.Get(
"/api/stats/summary",
[](const httplib::Request&, httplib::Response& res)
{


    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    int total_students = 0;

    int total_lecturers = 0;



    // Đếm sinh viên
    sqlite3_prepare_v2(
        db,
        "SELECT COUNT(*) FROM STUDENTS;",
        -1,
        &stmt,
        0
    );


    if(sqlite3_step(stmt)==SQLITE_ROW)
    {
        total_students =
        sqlite3_column_int(stmt,0);
    }


    sqlite3_finalize(stmt);




    // Đếm giảng viên
    sqlite3_prepare_v2(
        db,
        "SELECT COUNT(*) FROM LECTURERS;",
        -1,
        &stmt,
        0
    );


    if(sqlite3_step(stmt)==SQLITE_ROW)
    {
        total_lecturers =
        sqlite3_column_int(stmt,0);
    }


    sqlite3_finalize(stmt);



    sqlite3_close(db);




    std::string json =
    "{"
    "\"total_students\":"+
    std::to_string(total_students)
    +
    ","
    "\"total_lecturers\":"+
    std::to_string(total_lecturers)
    +
    "}";



    res.set_content(
        json,
        "application/json; charset=utf-8"
    );


});



}