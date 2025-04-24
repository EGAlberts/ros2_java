/* Copyright 2025 Elvin Alberts
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

import org.ros2.rcljava.RCLJava;
import java.util.ArrayList;

public class BetterComposableNode implements ComposableNode {
  private final String name;

  protected final Node node;

  public BetterComposableNode(String name, ArrayList<String> cli_args) {
    this.name = name;
    NodeOptions options = new NodeOptions();
    options.setCliArgs(cli_args);
    node = RCLJava.createNode(name, "", RCLJava.getDefaultContext(), options);
  }

  public Node getNode() {
    return node;
  }
}
