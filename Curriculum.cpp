#include "curriculum.h"
#include "sqlite3.h"
#include <string>


std::string safe_column_text_curriculum(sqlite3_stmt* stmt, int col)
{
    const char* text = (const char*)sqlite3_column_text(stmt, col);
    return text ? std::string(text) : "";
}


std::string get_json_value_curriculum(
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



void curriculum_routes(httplib::Server& svr)
{


// Lấy danh sách chương trình học
svr.Get(
"/api/curriculums",
[](const httplib::Request&, httplib::Response& res)
{


    sqlite3* db;
    sqlite3_open("student_system.db",&db);


    sqlite3_stmt* stmt;


    sqlite3_prepare_v2(
        db,
        "SELECT curriculum_id, major, academic_intake, total_credits FROM CURRICULUMS;",
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
        "\"id\":\"" + safe_column_text_curriculum(stmt,0) +
        "\",\"major\":\"" + safe_column_text_curriculum(stmt,1) +
        "\",\"intake\":\"" + safe_column_text_curriculum(stmt,2) +
        "\",\"total_credits\":" +
        std::to_string(sqlite3_column_int(stmt,3))
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




// Thêm chương trình học
svr.Post(
"/api/curriculums",
[](const httplib::Request& req,
httplib::Response& res)
{


    std::string id =
    get_json_value_curriculum(
        req.body,
        "curriculum_id"
    );


    std::string major =
    get_json_value_curriculum(
        req.body,
        "major"
    );


    std::string intake =
    get_json_value_curriculum(
        req.body,
        "academic_intake"
    );


    std::string credits =
    get_json_value_curriculum(
        req.body,
        "total_credits"
    );



    sqlite3* db;

    sqlite3_open(
        "student_system.db",
        &db
    );


    sqlite3_stmt* stmt;



    sqlite3_prepare_v2(
        db,
        "INSERT INTO CURRICULUMS "
        "(curriculum_id, major, academic_intake, total_credits)"
        " VALUES (?, ?, ?, ?);",
        -1,
        &stmt,
        0
    );



    sqlite3_bind_text(
        stmt,
        1,
        id.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_text(
        stmt,
        2,
        major.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_text(
        stmt,
        3,
        intake.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    sqlite3_bind_int(
        stmt,
        4,
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