/* Copyright 2020 Open Source Robotics Foundation, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.ros2.rcljava.node;

import static org.junit.Assert.assertEquals;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;

import org.ros2.rcljava.RCLJava;
import org.ros2.rcljava.node.Node;
import org.ros2.rcljava.node.NodeOptions;
import org.ros2.rcljava.parameters.*;


public class NodeOptionsTest {
  @BeforeClass
  public static void setupOnce() throws Exception {
    // Just to quiet down warnings
    try
    {
      // Configure log4j. Doing this dynamically so that Android does not complain about missing
      // the log4j JARs, SLF4J uses Android's native logging mechanism instead.
      Class c = Class.forName("org.apache.log4j.BasicConfigurator");
      Method m = c.getDeclaredMethod("configure", (Class<?>[]) null);
      Object o = m.invoke(null, (Object[]) null);
    }
    catch (Exception e)
    {
      e.printStackTrace();
    }

    RCLJava.rclJavaInit();
  }

  @AfterClass
  public static void tearDownOnce() {
    RCLJava.shutdown();
  }

  @Test
  public final void testCreateNodeWithArgs() {
    NodeOptions options = new NodeOptions();
    options.setCliArgs(new ArrayList<String>(Arrays.asList("--ros-args", "-r", "__ns:=/foo")));
    Node node = RCLJava.createNode("test_node", "", RCLJava.getDefaultContext(), options);
    assertEquals("test_node", node.getName());
    assertEquals("/foo", node.getNamespace());

    node.dispose();
  }

  @Test
  public final void testCreateNodeWithArgsOverrideParams() {
    NodeOptions options = new NodeOptions();
    
    options.setCliArgs(new ArrayList<String>(Arrays.asList("--ros-args",
    "-p", "int_param:=2",
    "-p", "bool_param:=false",
    "-p", "string_param:=hello",
    "-p", "double_param:=3.14",
    "-p", "string_array_param:=[hello,world]")));
    Node node = RCLJava.createNode("test_node", "", RCLJava.getDefaultContext(), options);
    
    node.declareParameter(new ParameterVariant("bool_param", true));
    node.declareParameter(new ParameterVariant("int_param", 1));
    node.declareParameter(new ParameterVariant("string_param", "world"));
    node.declareParameter(new ParameterVariant("double_param", 1.23));
    node.declareParameter(new ParameterVariant("string_array_param", new String[] {"foo", "bar"}));

    
    assertEquals(2,node.getParameter("int_param").asInt());
    assertEquals(false,node.getParameter("bool_param").asBool());
    assertEquals("hello",node.getParameter("string_param").asString());
    assertEquals(3.14,node.getParameter("double_param").asDouble(),0.01);
    assertEquals(new String[] {"hello", "world"}, node.getParameter("string_array_param").asStringArray());

    node.dispose();
  }
}
