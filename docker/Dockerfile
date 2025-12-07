FROM ros:humble-ros-core

RUN apt update \
    && DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends --no-install-suggests \
  openjdk-17-jdk \
  ros-dev-tools \
  wget \
  python3-vcstool \
  curl \
  python3-colcon-common-extensions \
  python3-pip \
  unzip \
  zip \
  && rm -rf /var/lib/apt/lists/* 

ENV JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64"
ENV PATH="$JAVA_HOME/bin:$PATH"

RUN pip install -U git+https://github.com/colcon/colcon-gradle
RUN pip install --no-deps -U git+https://github.com/colcon/colcon-ros-gradle

RUN curl -s "https://get.sdkman.io" | bash \
    && bash -c "source /root/.sdkman/bin/sdkman-init.sh && sdk install gradle 8.5"
    
ENV SDKMAN_DIR="/root/.sdkman"
ENV PATH="$SDKMAN_DIR/bin:$SDKMAN_DIR/candidates/gradle/current/bin:$PATH"    
    
WORKDIR /root/ros2_java_ws
RUN mkdir -p ~/ros2_java_ws/src
RUN curl -skL https://raw.githubusercontent.com/EGAlberts/ros2_java/refs/heads/humble-devel/ros2_java_desktop.repos | vcs import src
RUN git clone https://github.com/EGAlberts/ament_gradle_plugin.git --branch ivanpauno/gradle7.5-compatibility ./src/ament_gradle_plugin

RUN rosdep init
RUN apt update && apt upgrade -y \
    && rosdep update \
    && rosdep install -y --ignore-src --from-paths src -r

RUN cd src/ament_gradle_plugin && gradle publishToMavenLocal

    
RUN bash -c "source /opt/ros/humble/setup.bash && colcon build --symlink-install"
