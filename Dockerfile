FROM debian

# EXPOSE 8080

RUN mkdir /dsm
ADD build /dsm

# CMD /dsm/dsm/test/allocation1
