#include <iostream>
#include "httplib.h"


// Modules
#include "modules/Curriculum.h"
#include "modules/Course.h"
#include "modules/Class.h"
#include "modules/Enrollment.h"
#include "modules/Grade.h"
#include "modules/Report.h"


int main()
{
    httplib::Server svr;


    // Quản lý chương trình học
    registerCurriculumAPI(svr);


    // Quản lý môn học
    registerCourseAPI(svr);


    // Quản lý lớp học phần
    registerClassAPI(svr);


    // Ghi danh sinh viên
    registerEnrollmentAPI(svr);


    // Quản lý điểm
    registerGradeAPI(svr);


    // Báo cáo thống kê
    registerReportAPI(svr);



    std::cout << "=================================\n";
    std::cout << " Student Management System API\n";
    std::cout << " Server running at port 8080\n";
    std::cout << "=================================\n";


    svr.listen("0.0.0.0",8080);


    return 0;
}