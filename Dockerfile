FROM gcc:4.9
COPY . /home/devuser/Documents/cse3320
WORKDIR /home/devuser/Documents/cse3320
RUN gcc -o main main.c
CMD ["./main"]


