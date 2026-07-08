#include "course.h"
#include "sqlite3.h"
#include <string>


std::string safe_column_text_course(sqlite3_stmt* stmt, int col)
{
    const char* text = (const char*)sqlite3_column_text(stmt, col);
    return text ? std::string(text) : "";
}


std::string get_json_value_course(
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



void course_routes(httplib::Server& svr)
{


// Lấy danh sách môn học
svr.Get(
"/api/courses",
[](const httplib::Request&, httplib::Response& res)
{

    sqlite3* db;
    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;


    sqlite3_prepare_v2(
        db,
        "SELECT course_id, course_code, course_name, credits "
        "FROM COURSES;",
        -1,
        &stmt,
        0
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
        "\"id\":" +
        std::to_string(
            sqlite3_column_int(stmt,0)
        )
        +
        ",\"code\":\"" +
        safe_column_text_course(stmt,1)
        +
        "\",\"name\":\"" +
        safe_column_text_course(stmt,2)
        +
        "\",\"credits\":" +
        std::to_string(
            sqlite3_column_int(stmt,3)
        )
        +
        "}";

    }


    json += "]";


    sqlite3_finalize(stmt);

    sqlite3_close(db);



    res.set_content(
        json,
        "application/json; charset=utf-8"
    );


});




// Thêm môn học
svr.Post(
"/api/courses",
[](const httplib::Request& req,
httplib::Response& res)
{


    std::string code =
    get_json_value_course(
        req.body,
        "code"
    );


    std::string name =
    get_json_value_course(
        req.body,
        "name"
    );


    std::string credits =
    get_json_value_course(
        req.body,
        "credits"
    );



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "INSERT INTO COURSES "
        "(course_code, course_name, credits, semester, is_elective)"
        " VALUES (?, ?, ?, 1, 0);",
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


    sqlite3_bind_text(
        stmt,
        2,
        name.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_int(
        stmt,
        3,
        std::stoi(credits)
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