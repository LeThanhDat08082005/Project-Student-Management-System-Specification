FROM gcc:latest

WORKDIR /app

COPY . .

RUN gcc -c sqlite3.c -o sqlite3.o

RUN g++ -O3 -o server main.cpp sqlite3.o -pthread -ldl

EXPOSE 8080

CMD ["./server"]