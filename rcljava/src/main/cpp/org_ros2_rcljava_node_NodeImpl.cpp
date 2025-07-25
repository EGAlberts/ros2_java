// Copyright 2016-2018 Esteve Fernandez <esteve@apache.org>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <jni.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <regex>

#include "rcl_interfaces/msg/parameter_type.h"
#include "rcl_yaml_param_parser/parser.h"
#include "rcl/error_handling.h"
#include "rcl/graph.h"
#include "rcl/node.h"
#include "rcl/rcl.h"
#include "rcpputils/scope_exit.hpp"
#include <rcpputils/find_and_replace.hpp>
#include "rmw/rmw.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

#include "rcljava_common/exceptions.hpp"
#include "rcljava_common/signatures.hpp"

#include "org_ros2_rcljava_node_NodeImpl.h"

using rcljava_common::exceptions::rcljava_throw_exception;
using rcljava_common::exceptions::rcljava_throw_rclexception;

JNIEXPORT jstring JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetName(
  JNIEnv * env, jclass, jlong node_handle)
{
  return env->NewStringUTF(rcl_node_get_name(reinterpret_cast<rcl_node_t *>(node_handle)));
}

JNIEXPORT jstring JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetNamespace(
  JNIEnv * env, jclass, jlong node_handle)
{
  return env->NewStringUTF(rcl_node_get_namespace(reinterpret_cast<rcl_node_t *>(node_handle)));
}

JNIEXPORT jlong JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeCreatePublisherHandle(
  JNIEnv * env, jclass, jlong node_handle, jclass jmessage_class, jstring jtopic,
  jlong qos_profile_handle)
{
  jmethodID mid = env->GetStaticMethodID(jmessage_class, "getTypeSupport", "()J");
  jlong jts = env->CallStaticLongMethod(jmessage_class, mid);

  const char * topic = env->GetStringUTFChars(jtopic, 0);

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rosidl_message_type_support_t * ts = reinterpret_cast<rosidl_message_type_support_t *>(jts);

  rcl_publisher_t * publisher = static_cast<rcl_publisher_t *>(malloc(sizeof(rcl_publisher_t)));
  *publisher = rcl_get_zero_initialized_publisher();
  rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();

  rmw_qos_profile_t * qos_profile = reinterpret_cast<rmw_qos_profile_t *>(qos_profile_handle);
  publisher_ops.qos = *qos_profile;

  rcl_ret_t ret = rcl_publisher_init(publisher, node, ts, topic, &publisher_ops);
  env->ReleaseStringUTFChars(jtopic, topic);

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to create publisher: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
    return 0;
  }

  jlong jpublisher = reinterpret_cast<jlong>(publisher);
  return jpublisher;
}

JNIEXPORT jlong JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeCreateSubscriptionHandle(
  JNIEnv * env, jclass, jlong node_handle, jclass jmessage_class, jstring jtopic,
  jlong qos_profile_handle)
{
  jmethodID mid = env->GetStaticMethodID(jmessage_class, "getTypeSupport", "()J");
  jlong jts = env->CallStaticLongMethod(jmessage_class, mid);

  const char * topic = env->GetStringUTFChars(jtopic, 0);

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rosidl_message_type_support_t * ts = reinterpret_cast<rosidl_message_type_support_t *>(jts);

  rcl_subscription_t * subscription =
    static_cast<rcl_subscription_t *>(malloc(sizeof(rcl_subscription_t)));
  *subscription = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_ops = rcl_subscription_get_default_options();

  rmw_qos_profile_t * qos_profile = reinterpret_cast<rmw_qos_profile_t *>(qos_profile_handle);
  subscription_ops.qos = *qos_profile;

  rcl_ret_t ret = rcl_subscription_init(subscription, node, ts, topic, &subscription_ops);
  env->ReleaseStringUTFChars(jtopic, topic);

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to create subscription: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
    return 0;
  }

  jlong jsubscription = reinterpret_cast<jlong>(subscription);
  return jsubscription;
}

JNIEXPORT jlong JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeCreateServiceHandle(
  JNIEnv * env, jclass, jlong node_handle, jclass jservice_class, jstring jservice_name,
  jlong qos_profile_handle)
{
  jmethodID mid = env->GetStaticMethodID(jservice_class, "getServiceTypeSupport", "()J");

  assert(mid != NULL);

  jlong jts = env->CallStaticLongMethod(jservice_class, mid);

  assert(jts != 0);

  const char * service_name = env->GetStringUTFChars(jservice_name, 0);

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rosidl_service_type_support_t * ts = reinterpret_cast<rosidl_service_type_support_t *>(jts);

  rcl_service_t * service = static_cast<rcl_service_t *>(malloc(sizeof(rcl_service_t)));
  *service = rcl_get_zero_initialized_service();
  rcl_service_options_t service_ops = rcl_service_get_default_options();

  rmw_qos_profile_t * qos_profile = reinterpret_cast<rmw_qos_profile_t *>(qos_profile_handle);
  service_ops.qos = *qos_profile;

  rcl_ret_t ret = rcl_service_init(service, node, ts, service_name, &service_ops);
  env->ReleaseStringUTFChars(jservice_name, service_name);

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to create service: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
    return 0;
  }

  jlong jservice = reinterpret_cast<jlong>(service);
  return jservice;
}

JNIEXPORT jlong JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeCreateClientHandle(
  JNIEnv * env, jclass, jlong node_handle, jclass jservice_class, jstring jservice_name,
  jlong qos_profile_handle)
{
  jmethodID mid = env->GetStaticMethodID(jservice_class, "getServiceTypeSupport", "()J");

  assert(mid != NULL);

  jlong jts = env->CallStaticLongMethod(jservice_class, mid);

  assert(jts != 0);

  const char * service_name = env->GetStringUTFChars(jservice_name, 0);

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rosidl_service_type_support_t * ts = reinterpret_cast<rosidl_service_type_support_t *>(jts);

  rcl_client_t * client = static_cast<rcl_client_t *>(malloc(sizeof(rcl_client_t)));
  *client = rcl_get_zero_initialized_client();
  rcl_client_options_t client_ops = rcl_client_get_default_options();

  rmw_qos_profile_t * qos_profile = reinterpret_cast<rmw_qos_profile_t *>(qos_profile_handle);
  client_ops.qos = *qos_profile;

  rcl_ret_t ret = rcl_client_init(client, node, ts, service_name, &client_ops);
  env->ReleaseStringUTFChars(jservice_name, service_name);

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to create client: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
    return 0;
  }

  jlong jclient = reinterpret_cast<jlong>(client);
  return jclient;
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeDispose(JNIEnv * env, jclass, jlong node_handle)
{
  if (node_handle == 0) {
    // already destroyed
    return;
  }

  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(node_handle);

  rcl_ret_t ret = rcl_node_fini(node);

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to destroy node: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
  }
}

JNIEXPORT jlong JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeCreateTimerHandle(
  JNIEnv * env, jclass, jlong clock_handle, jlong context_handle, jlong timer_period)
{
  rcl_clock_t * clock = reinterpret_cast<rcl_clock_t *>(clock_handle);
  rcl_context_t * context = reinterpret_cast<rcl_context_t *>(context_handle);

  rcl_timer_t * timer = static_cast<rcl_timer_t *>(malloc(sizeof(rcl_timer_t)));
  *timer = rcl_get_zero_initialized_timer();

  rcl_ret_t ret = rcl_timer_init(
    timer, clock, context, timer_period, NULL, rcl_get_default_allocator());

  if (ret != RCL_RET_OK) {
    std::string msg = "Failed to create timer: " + std::string(rcl_get_error_string().str);
    rcl_reset_error();
    rcljava_throw_rclexception(env, ret, msg);
    return 0;
  }

  jlong jtimer = reinterpret_cast<jlong>(timer);
  return jtimer;
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetNodeNames(
  JNIEnv * env, jclass, jlong handle, jobject jnode_names_info)
{
  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(handle);
  if (!node) {
    rcljava_throw_exception(env, "java/lang/IllegalArgumentException", "node handle is NULL");
    return;
  }

  jclass list_clazz = env->GetObjectClass(jnode_names_info);
  jmethodID list_add_mid = env->GetMethodID(list_clazz, "add", "(Ljava/lang/Object;)Z");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jclass node_info_clazz = env->FindClass("org/ros2/rcljava/graph/NodeNameInfo");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jmethodID node_info_init_mid = env->GetMethodID(node_info_clazz, "<init>", "()V");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jfieldID name_fid = env->GetFieldID(node_info_clazz, "name", "Ljava/lang/String;");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jfieldID namespace_fid = env->GetFieldID(node_info_clazz, "namespace", "Ljava/lang/String;");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jfieldID enclave_fid = env->GetFieldID(node_info_clazz, "enclave", "Ljava/lang/String;");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t enclaves = rcutils_get_zero_initialized_string_array();

  rcl_ret_t ret = rcl_get_node_names_with_enclaves(
    node,
    allocator,
    &node_names,
    &node_namespaces,
    &enclaves);
  RCLJAVA_COMMON_THROW_FROM_RCL(env, ret, "rcl_get_node_names_with_enclaves failed");
  auto on_scope_exit = rcpputils::make_scope_exit(
    [pnames = &node_names, pnamespaces = &node_namespaces, penclaves = &enclaves, env]() {
      rcl_ret_t ret = rcutils_string_array_fini(pnames);
      if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
        rcljava_throw_rclexception(env, ret, "failed to fini node names string array");
      }
      ret = rcutils_string_array_fini(pnamespaces);
      if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
        rcljava_throw_rclexception(env, ret, "failed to fini node namespaces string array");
      }
      ret = rcutils_string_array_fini(penclaves);
      if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
        rcljava_throw_rclexception(env, ret, "failed to fini enclaves string array");
      }
    }
  );

  if (node_names.size != node_namespaces.size || node_names.size != enclaves.size) {
    rcljava_throw_exception(
      env,
      "java/lang/IllegalStateException",
      "names, namespaces and enclaves array leghts don't match");
    return;
  }

  for (size_t i = 0; i < node_names.size; i++) {
    jstring jnode_name = env->NewStringUTF(node_names.data[i]);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    jstring jnode_namespace = env->NewStringUTF(node_namespaces.data[i]);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    jstring jenclave = env->NewStringUTF(enclaves.data[i]);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    jobject jitem = env->NewObject(node_info_clazz, node_info_init_mid);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    env->SetObjectField(jitem, name_fid, jnode_name);
    env->SetObjectField(jitem, namespace_fid, jnode_namespace);
    env->SetObjectField(jitem, enclave_fid, jenclave);
    env->CallBooleanMethod(jnode_names_info, list_add_mid, jitem);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  }
}

void
fill_jnames_and_types(
  JNIEnv * env, const rcl_names_and_types_t & names_and_types, jobject jnames_and_types)
{
  jclass collection_clazz = env->FindClass("java/util/Collection");
  jmethodID collection_add_mid = env->GetMethodID(
    collection_clazz, "add", "(Ljava/lang/Object;)Z");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jclass name_and_types_clazz = env->FindClass("org/ros2/rcljava/graph/NameAndTypes");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jmethodID name_and_types_init_mid = env->GetMethodID(name_and_types_clazz, "<init>", "()V");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jfieldID name_fid = env->GetFieldID(name_and_types_clazz, "name", "Ljava/lang/String;");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jfieldID types_fid = env->GetFieldID(name_and_types_clazz, "types", "Ljava/util/Collection;");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

  for (size_t i = 0; i < names_and_types.names.size; i++) {
    jobject jitem = env->NewObject(name_and_types_clazz, name_and_types_init_mid);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    jstring jname = env->NewStringUTF(names_and_types.names.data[i]);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    env->SetObjectField(jitem, name_fid, jname);
    // the default constructor already inits types to an empty ArrayList
    jobject jtypes = env->GetObjectField(jitem, types_fid);
    for (size_t j = 0; j < names_and_types.types[i].size; j++) {
      jstring jtype = env->NewStringUTF(names_and_types.types[i].data[j]);
      env->CallBooleanMethod(jtypes, collection_add_mid, jtype);
      RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    }
    env->CallBooleanMethod(jnames_and_types, collection_add_mid, jitem);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  }
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetTopicNamesAndTypes(
  JNIEnv * env, jclass, jlong handle, jobject jnames_and_types)
{
  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(handle);
  if (!node) {
    rcljava_throw_exception(env, "java/lang/IllegalArgumentException", "node handle is NULL");
    return;
  }

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_names_and_types_t topic_names_and_types = rcl_get_zero_initialized_names_and_types();

  rcl_ret_t ret = rcl_get_topic_names_and_types(
    node,
    &allocator,
    false,
    &topic_names_and_types);
  RCLJAVA_COMMON_THROW_FROM_RCL(env, ret, "failed to get topic names and types");
  fill_jnames_and_types(env, topic_names_and_types, jnames_and_types);

  ret = rcl_names_and_types_fini(&topic_names_and_types);
  if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
    rcljava_throw_rclexception(env, ret, "failed to fini topic names and types structure");
  }
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetServiceNamesAndTypes(
  JNIEnv * env, jclass, jlong handle, jobject jnames_and_types)
{
  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(handle);
  if (!node) {
    rcljava_throw_exception(env, "java/lang/IllegalArgumentException", "node handle is NULL");
    return;
  }

  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_names_and_types_t service_names_and_types = rcl_get_zero_initialized_names_and_types();

  rcl_ret_t ret = rcl_get_service_names_and_types(
    node,
    &allocator,
    &service_names_and_types);
  RCLJAVA_COMMON_THROW_FROM_RCL(env, ret, "failed to get service names and types");
  fill_jnames_and_types(env, service_names_and_types, jnames_and_types);

  ret = rcl_names_and_types_fini(&service_names_and_types);
  if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
    rcljava_throw_rclexception(env, ret, "failed to fini service names and types structure");
  }
}

template<typename FunctorT>
void
get_endpoint_info_common(
  JNIEnv * env, jlong handle, jstring jtopic_name, jobject jendpoints_info, FunctorT get_info)
{
  rcl_node_t * node = reinterpret_cast<rcl_node_t *>(handle);
  if (!node) {
    rcljava_throw_exception(
      env, "java/lang/IllegalArgumentException", "passed node handle is NULL");
    return;
  }

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcl_topic_endpoint_info_array_t endpoints_info =
    rcl_get_zero_initialized_topic_endpoint_info_array();

  const char * topic_name = env->GetStringUTFChars(jtopic_name, NULL);
  if (!topic_name) {
    rcljava_throw_exception(
      env, "java/lang/IllegalArgumentException", "failed to convert jstring to utf chars");
    return;
  }

  rcl_ret_t ret = get_info(
    node,
    &allocator,
    topic_name,
    false,  // use ros mangling conventions
    &endpoints_info);

  env->ReleaseStringUTFChars(jtopic_name, topic_name);

  RCLJAVA_COMMON_THROW_FROM_RCL(env, ret, "failed to get publisher info");
  auto cleanup_info_array = rcpputils::make_scope_exit(
    [info_ptr = &endpoints_info, allocator_ptr = &allocator, env]() {
      rcl_ret_t ret = rcl_topic_endpoint_info_array_fini(info_ptr, allocator_ptr);
      if (!env->ExceptionCheck() && RCL_RET_OK != ret) {
        rcljava_throw_rclexception(env, ret, "failed to destroy rcl endpoints info");
      }
    }
  );

  jclass list_clazz = env->GetObjectClass(jendpoints_info);
  jmethodID list_add_mid = env->GetMethodID(list_clazz, "add", "(Ljava/lang/Object;)Z");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jclass endpoint_info_clazz = env->FindClass("org/ros2/rcljava/graph/EndpointInfo");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jmethodID endpoint_info_init_mid = env->GetMethodID(endpoint_info_clazz, "<init>", "()V");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
  jmethodID endpoint_info_from_rcl_mid = env->GetMethodID(
    endpoint_info_clazz, "nativeFromRCL", "(J)V");
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

  for (size_t i = 0; i < endpoints_info.size; i++) {
    jobject item = env->NewObject(endpoint_info_clazz, endpoint_info_init_mid);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    env->CallVoidMethod(item, endpoint_info_from_rcl_mid, &endpoints_info.info_array[i]);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    env->CallBooleanMethod(jendpoints_info, list_add_mid, item);
    RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);
    env->DeleteLocalRef(item);
  }
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetPublishersInfo(
  JNIEnv * env, jclass, jlong handle, jstring jtopic_name, jobject jpublishers_info)
{
  get_endpoint_info_common(
    env, handle, jtopic_name, jpublishers_info, rcl_get_publishers_info_by_topic);
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetSubscriptionsInfo(
  JNIEnv * env, jclass, jlong handle, jstring jtopic_name, jobject jsubscriptions_info)
{
  get_endpoint_info_common(
    env, handle, jtopic_name, jsubscriptions_info, rcl_get_subscriptions_info_by_topic);
}

/// Create an rclpy.parameter.Parameter from an rcl_variant_t
/**
 * \param[in] jname name of the parameter
 * \param[in] variant a variant to create a Parameter from
 * \return an instance of ParameterVariant
 */
jobject
_parameter_from_rcl_variant(JNIEnv * env, jstring jname, rcl_variant_t * variant)
{
  jobject parameter = nullptr;
  jclass parameter_variant_cls = env->FindClass("org/ros2/rcljava/parameters/ParameterVariant");
  if (variant->bool_value) {
    jmethodID parameter_variant_init_mid = env->GetMethodID(
      parameter_variant_cls,
      "<init>", "(Ljava/lang/String;Z)V");

    parameter = env->NewObject(
      parameter_variant_cls, parameter_variant_init_mid,
      jname, *(variant->bool_value));
  } else if (variant->integer_value) {
    jmethodID parameter_variant_init_mid = env->GetMethodID(
      parameter_variant_cls, "<init>",
      "(Ljava/lang/String;I)V");
    parameter = env->NewObject(
      parameter_variant_cls, parameter_variant_init_mid,
      jname, *(variant->integer_value));
  } else if (variant->double_value) {
    jmethodID parameter_variant_init_mid = env->GetMethodID(
      parameter_variant_cls, "<init>",
      "(Ljava/lang/String;D)V");
    parameter = env->NewObject(
      parameter_variant_cls, parameter_variant_init_mid,
      jname, *(variant->double_value));
  } else if (variant->string_value) {
    jmethodID parameter_variant_init_mid = env->GetMethodID(
      parameter_variant_cls, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;)V");

    jstring jvalue = env->NewStringUTF(variant->string_value);

    parameter = env->NewObject(
      parameter_variant_cls, parameter_variant_init_mid,
      jname, jvalue);
  } else if (variant->byte_array_value) {
    rcljava_throw_exception(env, "1", "NotYetImplemented");
    // value = py::bytes(
    //   reinterpret_cast<char *>(variant->byte_array_value->values),
    //   variant->byte_array_value->size);
  } else if (variant->bool_array_value) {
    rcljava_throw_exception(env, "1", "NotYetImplemented");
    // py::list list_value = py::list(variant->bool_array_value->size);
    // for (size_t i = 0; i < variant->bool_array_value->size; ++i) {
    //   list_value[i] = py::bool_(variant->bool_array_value->values[i]);
    // }
    // value = list_value;
  } else if (variant->integer_array_value) {
    rcljava_throw_exception(env, "1", "NotYetImplemented");
    // py::list list_value = py::list(variant->integer_array_value->size);
    // for (size_t i = 0; i < variant->integer_array_value->size; ++i) {
    //   list_value[i] = py::int_(variant->integer_array_value->values[i]);
    // }
    // value = list_value;
  } else if (variant->double_array_value) {
    rcljava_throw_exception(env, "1", "NotYetImplemented");
    // py::list list_value = py::list(variant->double_array_value->size);
    // for (size_t i = 0; i < variant->double_array_value->size; ++i) {
    //   list_value[i] = py::float_(variant->double_array_value->values[i]);
    // }
    // value = list_value;
  } else if (variant->string_array_value) {
    jmethodID parameter_variant_init_mid = env->GetMethodID(
      parameter_variant_cls, "<init>",
      "(Ljava/lang/String;[Ljava/lang/String;)V");
    if (parameter_variant_init_mid == nullptr) {
      rcljava_throw_exception(
        env,
        "java/lang/NoSuchMethodException",
        "ParameterVariant constructor for string arrays not found");
      RCLJAVA_COMMON_CHECK_FOR_EXCEPTION_WITH_ERROR_STATEMENT(env, "boo!");
      return nullptr;
    }

    size_t array_size = variant->string_array_value->size;
    jobjectArray jstring_array = env->NewObjectArray(
      array_size, env->FindClass("java/lang/String"), nullptr);
    if (jstring_array == nullptr) {
      rcljava_throw_exception(
        env,
        "java/lang/OutOfMemoryError",
        "Failed to create Java string array");
      RCLJAVA_COMMON_CHECK_FOR_EXCEPTION_WITH_ERROR_STATEMENT(env, "boo!");
      return nullptr;
    }

    for (size_t i = 0; i < array_size; ++i) {
      jstring jvalue = env->NewStringUTF(variant->string_array_value->data[i]);
      if (jvalue == nullptr) {
        rcljava_throw_exception(env, "java/lang/OutOfMemoryError", "Failed to create Java string");
        RCLJAVA_COMMON_CHECK_FOR_EXCEPTION_WITH_ERROR_STATEMENT(env, "boo!");
        return nullptr;
      }
      env->SetObjectArrayElement(jstring_array, i, jvalue);
      env->DeleteLocalRef(jvalue);  // Clean up local reference
    }

    parameter = env->NewObject(
      parameter_variant_cls, parameter_variant_init_mid, jname, jstring_array);
    if (env->ExceptionCheck()) {
      return nullptr;
    }

    env->DeleteLocalRef(jstring_array);
  }

  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION_WITH_ERROR_STATEMENT(env, "boo!");

  return parameter;
}


void
_populate_node_parameters_from_rcl_params(
  JNIEnv * env,
  const rcl_params_t * params,
  jobject jnode_parameters_map,  // Java Map<String, List<ParameterVariant>>
  const char * node_fqn)
{
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);


  for (size_t i = 0; i < params->num_nodes; ++i) {
    std::string node_name{params->node_names[i]};
    if (node_name.empty()) {
      rcljava_throw_exception(env, "1", "expected node name to have at least one character");
    }

    // Make sure all node names start with '/'
    if ('/' != node_name.front()) {
      node_name.insert(node_name.begin(), '/');
    }

    if (node_fqn) {
      // Update the regular expression ["/*" -> "(/\\w+)" and "/**" -> "(/\\w+)*"]
      std::string regex = rcpputils::find_and_replace(node_name, "/*", "(/\\w+)");
      if (!std::regex_match(node_fqn, std::regex(regex))) {
        // No need to parse the items because the user just care about node_fqn
        continue;
      }

      node_name = node_fqn;
    }

    // Convert the node name to a Java string
    jstring jnode_name = env->NewStringUTF(node_name.c_str());

    // Check if the node already exists in the Java Map
    jclass map_class = env->GetObjectClass(jnode_parameters_map);
    jmethodID contains_key_method = env->GetMethodID(
      map_class,
      "containsKey", "(Ljava/lang/Object;)Z");
    jboolean node_exists = env->CallBooleanMethod(
      jnode_parameters_map,
      contains_key_method, jnode_name);


    jobject parameter_list;

    if (!node_exists) {
      // Create a new list for the node's parameters
      jclass array_list_class = env->FindClass("java/util/ArrayList");
      jmethodID array_list_init = env->GetMethodID(array_list_class, "<init>", "()V");
      jobject new_parameter_list = env->NewObject(array_list_class, array_list_init);

      // Add the new list to the map
      jmethodID put_method = env->GetMethodID(
        map_class, "put",
        "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
      env->CallObjectMethod(jnode_parameters_map, put_method, jnode_name, new_parameter_list);

      // Clean up local reference for the new list
      env->DeleteLocalRef(new_parameter_list);
    }

    jmethodID get_method = env->GetMethodID(
      map_class, "get",
      "(Ljava/lang/Object;)Ljava/lang/Object;");
    parameter_list = env->CallObjectMethod(jnode_parameters_map, get_method, jnode_name);


    // Process the parameters for the current node
    rcl_node_params_t node_params = params->params[i];
    for (size_t j = 0; j < node_params.num_params; ++j) {
      // Convert the parameter name to a Java string
      jstring jparam_name = env->NewStringUTF(node_params.parameter_names[j]);

      // Call the helper function to create a ParameterVariant object
      jobject parameter_variant = _parameter_from_rcl_variant(
        env, jparam_name, &node_params.parameter_values[j]);

      // Add the ParameterVariant to the list
      if (parameter_list == nullptr) {
        rcljava_throw_exception(env, "java/lang/IllegalStateException", "parameter_list is null");
      }
      RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);


      jclass list_class = env->GetObjectClass(parameter_list);
      RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

      jmethodID add_method = env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z");
      env->CallBooleanMethod(parameter_list, add_method, parameter_variant);

      // Clean up local references
      env->DeleteLocalRef(jparam_name);
      env->DeleteLocalRef(parameter_variant);
    }

    // Clean up local reference for the node name
    env->DeleteLocalRef(jnode_name);
  }
}


/// Populate a Java list with node parameters parsed from CLI arguments
/**
 * \param[in] args CLI arguments to parse for parameters
 * \param[in] node_fqn the FQN of node
 * \param[out] params_by_node_name A Python dict object to place parsed parameters into.
 */
void
_parse_param_overrides(
  JNIEnv * env,
  const rcl_arguments_t * args, jobject jnode_parameters_map,
  const char * node_fqn)
{
  rcl_params_t * params = nullptr;
  if (RCL_RET_OK != rcl_arguments_get_param_overrides(args, &params)) {
    rcljava_throw_exception(env, "1", "failed to get parameter overrides");
  }
  if (params) {
    RCPPUTILS_SCOPE_EXIT({rcl_yaml_node_struct_fini(params);});
    _populate_node_parameters_from_rcl_params(
      env, params, jnode_parameters_map, node_fqn);
  }
}

JNIEXPORT void JNICALL
Java_org_ros2_rcljava_node_NodeImpl_nativeGetParameters(
  JNIEnv * env, jclass, jlong handle, jobject jparameters)
{
  const rcl_node_options_t * node_options =
    rcl_node_get_options(reinterpret_cast<rcl_node_t *>(handle));

  const char * node_fqn = rcl_node_get_fully_qualified_name(reinterpret_cast<rcl_node_t *>(handle));
  if (!node_fqn) {
    rcljava_throw_exception(env, "1", "failed to get node fully qualified name");
  }
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

  // Create a new HashMap for jnode_parameters_map
  jclass hhash_map_class = env->FindClass("java/util/HashMap");
  if (hhash_map_class == nullptr) {
    rcljava_throw_exception(
      env, "java/lang/ClassNotFoundException",
      "java.util.HashMap class not found");
    return;
  }
  RCLJAVA_COMMON_CHECK_FOR_EXCEPTION(env);

  jclass hash_map_class = env->FindClass("java/util/HashMap");
  jmethodID hash_map_init = env->GetMethodID(hash_map_class, "<init>", "()V");
  jobject jnode_parameters_map = env->NewObject(hash_map_class, hash_map_init);

  if (node_options->use_global_arguments) {
    _parse_param_overrides(
      env,
      &(reinterpret_cast<rcl_node_t *>(handle)->context->global_arguments),
      jnode_parameters_map, node_fqn);
  }

  _parse_param_overrides(
    env,
    &(node_options->arguments),
    jnode_parameters_map, node_fqn);


  // Access the parameters for the current node from the map
  jclass map_class = env->GetObjectClass(jnode_parameters_map);
  jmethodID get_method = env->GetMethodID(
    map_class, "get",
    "(Ljava/lang/Object;)Ljava/lang/Object;");

  // Convert the node FQN to a Java string
  jstring jnode_fqn = env->NewStringUTF(node_fqn);

  // Retrieve the list of parameters for the node
  jobject parameter_list = env->CallObjectMethod(jnode_parameters_map, get_method, jnode_fqn);

  // Check if the list is not null
  if (parameter_list != nullptr) {
    // Add all parameters from the list to jparameters
    jclass list_class = env->GetObjectClass(jparameters);
    jmethodID add_all_method = env->GetMethodID(list_class, "addAll", "(Ljava/util/Collection;)Z");
    env->CallBooleanMethod(jparameters, add_all_method, parameter_list);
  }

  // Clean up local references
  env->DeleteLocalRef(jnode_fqn);
}
