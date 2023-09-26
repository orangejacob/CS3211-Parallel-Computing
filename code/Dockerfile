# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

# Update apps on the base image
RUN apt-get -y update && apt-get install -y
RUN apt-get update && apt-get install make
RUN apt install nano

# Install the Clang compiler
RUN apt-get -y install clang

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/dockertest1

# Specify the working directory
WORKDIR /usr/src/dockertest1

# Use Clang to compile the Test.cpp source file
#RUN clang++ -o Test Test.cpp
RUN chmod +x test_cancel.sh
RUN chmod +x test_order.sh
RUN chmod +x test_random.sh
RUN make
#RUN ./engine socket
# Run the output program from the previous step
#CMD ["ls", "make", "./engine socket"]
