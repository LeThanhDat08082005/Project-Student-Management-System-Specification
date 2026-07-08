#include "grade.h"
#include "sqlite3.h"
#include <string>


std::string safe_column_text_grade(sqlite3_stmt* stmt, int col)
{
    const char* text = (const char*)sqlite3_column_text(stmt, col);
    return text ? std::string(text) : "";
}


std::string get_json_value_grade(
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
        num_end-num_start
    );
}



void grade_routes(httplib::Server& svr)
{


// ========================
// XEM ĐIỂM THEO LỚP HỌC PHẦN
// ========================


svr.Get(
"/api/grades",
[](const httplib::Request& req,
httplib::Response& res)
{


    std::string class_id =
    req.get_param_value("class_id");



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "SELECT "
        "E.enroll_id,"
        "S.student_code,"
        "S.full_name,"
        "E.process_grade,"
        "E.mid_grade,"
        "E.final_grade "
        "FROM ENROLLMENTS E "
        "JOIN STUDENTS S "
        "ON E.student_id=S.student_id "
        "WHERE E.class_id=?;",
        -1,
        &stmt,
        0
    );



    sqlite3_bind_text(
        stmt,
        1,
        class_id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );



    std::string json="[";
    bool first=true;



    while(sqlite3_step(stmt)==SQLITE_ROW)
    {

        if(!first)
            json += ",";

        first=false;



        json +=
        "{"
        "\"enroll_id\":"+
        std::to_string(
            sqlite3_column_int(stmt,0)
        )
        +
        ",\"student_code\":\""+
        safe_column_text_grade(stmt,1)
        +
        "\",\"student_name\":\""+
        safe_column_text_grade(stmt,2)
        +
        "\",\"process_grade\":"+
        std::to_string(
            sqlite3_column_double(stmt,3)
        )
        +
        ",\"mid_grade\":"+
        std::to_string(
            sqlite3_column_double(stmt,4)
        )
        +
        ",\"final_grade\":"+
        std::to_string(
            sqlite3_column_double(stmt,5)
        )
        +
        "}";

    }


    json+="]";



    sqlite3_finalize(stmt);

    sqlite3_close(db);



    res.set_content(
        json,
        "application/json; charset=utf-8"
    );


});





// ========================
// CẬP NHẬT ĐIỂM
// ========================


svr.Post(
"/api/update_grade",
[](const httplib::Request& req,
httplib::Response& res)
{


    std::string enroll_id =
    get_json_value_grade(
        req.body,
        "enroll_id"
    );


    std::string process =
    get_json_value_grade(
        req.body,
        "process"
    );


    std::string mid =
    get_json_value_grade(
        req.body,
        "mid"
    );


    std::string final =
    get_json_value_grade(
        req.body,
        "final"
    );



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );



    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "UPDATE ENROLLMENTS SET "
        "process_grade=?,"
        "mid_grade=?,"
        "final_grade=? "
        "WHERE enroll_id=?;",
        -1,
        &stmt,
        0
    );



    sqlite3_bind_double(
        stmt,
        1,
        std::stod(process)
    );


    sqlite3_bind_double(
        stmt,
        2,
        std::stod(mid)
    );


    sqlite3_bind_double(
        stmt,
        3,
        std::stod(final)
    );


    sqlite3_bind_int(
        stmt,
        4,
        std::stoi(enroll_id)
    );



    sqlite3_step(stmt);



    sqlite3_finalize(stmt);

    sqlite3_close(db);



    res.status=200;


    res.set_content(
        "{\"status\":\"success\"}",
        "application/json"
    );


});



}