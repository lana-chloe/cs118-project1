# CS118 Project 0 - Lana Lim 105817312
This is the repo for spring23 cs118 project 0.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Provided Files

- `project` is folder to develop codes for future projects.
- `docker-compose.yaml` and `Dockerfile` are files configuring the containers.

## Bash commands

```bash
# Setup the container(s) (make setup)
docker compose up -d

# Bash into the container (make shell)
docker compose exec node1 bash

# Remove container(s) and the Docker image (make clean)
docker compose down -v --rmi all --remove-orphans
```

## Environment

- OS: ubuntu 22.04
- IP: 192.168.10.225. NOT accessible from the host machine.
- Port forwarding: container 8080 <-> host 8080
  - Use http://localhost:8080 to access the HTTP server in the container.
- Files in this repo are in the `/project` folder. That means, `server.c` is `/project/project/server.c` in the container.

## PROJECT REPORT
Problems I ran into...

I struggled a bit with learning how to manipulate c strings, replace substrings, use char pointers in c, etc.. I brushed up on these concepts and ended up using char pointers to parse the HTTP request and malloc to hardcode the string the pointers point to in memory.

I didn't know the URL encoded spaces with %20 at first, so I had to go back and implement some code that takes the parsed string from the HTTP request and replace every instance of "%20" with ' '.

Similarly, I didn't know the URL encodes '%' with "%25", so I had to go back and fix that as well.

I was stuck on the 404 error code http response for a bit. At first I was sending an error.html file in my local directory to display the "404 error Not Found" text on the server, but ultimately fixed it by sending the literal string equivalent of that html file to the client since error.html would not be used/available in the testing environment.

Acknowledgements:
- Beej’s Guide to Network Programming
