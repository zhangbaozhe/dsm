FROM ubuntu

RUN apt update && apt install -y python3
RUN mkdir /dsm & mkdir /web
ADD build /dsm
ADD dsm/web /web

