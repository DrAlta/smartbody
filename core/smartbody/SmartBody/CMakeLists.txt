include_directories( ../smartbody-dll/include )
include_directories( ../smartbody-lib/src)
include_directories( /usr/include )

add_definitions( "-DdDOUBLE" )

set(SB_EXPORTS 1)
set(SMARTBODY_DLL_EXPORTS 1)
set(SMARTBODY_C_DLL_EXPORTS 1)
set(_COMPILING_STEERLIB 1)


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOUNIT_LIBRARY AudioUnit)
find_library (CORESERVICES_LIBRARY CoreServices)
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
find_library (GLEW GLEW)
find_library (FESTIVAL Festival ../../../lib/festival/festival/src/lib)
find_library (ESTBASE estbase ../../../lib/festival/speech_tools/lib)
find_library (ESTSTRING eststring ../../..//lib/festival/speech_tools/lib)
find_library (ESTOOLS estools ../../../lib/festival/speech_tools/lib)
else()
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
endif()

#if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#	if (${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.2.0)
#		add_definitions(-Wno-unused-value)
#	endif()
#endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set( MORE_LIBS
	${COREAUDIO_LIBRARY}
	${AUDIOUNIT_LIBRARY}
	${CORESERVICES_LIBRARY}
	${LIBSNDFILE}
	${ALUT}
	${OPENAL}
	${GLEW}
	${FESTIVAL}
	${ESTBASE}
	${ESTSTRING}
	${ESTOOLS}
)
else()
set( MORE_LIBS
	${LIBSNDFILE}
	${ALUT}
	${OPENAL}
	rt
)       
endif()


find_package(OpenGL REQUIRED)


set( SMARTBODY_LIB_EXT_ANN_SRC

        ../smartbody-lib/src/external/ANN/ANN.cpp
        ../smartbody-lib/src/external/ANN/bd_fix_rad_search.cpp
        ../smartbody-lib/src/external/ANN/bd_pr_search.cpp
        ../smartbody-lib/src/external/ANN/bd_search.cpp
        ../smartbody-lib/src/external/ANN/bd_tree.cpp
        ../smartbody-lib/src/external/ANN/brute.cpp
        ../smartbody-lib/src/external/ANN/kd_dump.cpp
        ../smartbody-lib/src/external/ANN/kd_fix_rad_search.cpp
        ../smartbody-lib/src/external/ANN/kd_pr_search.cpp
        ../smartbody-lib/src/external/ANN/kd_search.cpp
        ../smartbody-lib/src/external/ANN/kd_split.cpp
        ../smartbody-lib/src/external/ANN/kd_tree.cpp
        ../smartbody-lib/src/external/ANN/kd_util.cpp
        ../smartbody-lib/src/external/ANN/perf.cpp

)

set( SMARTBODY_LIB_EXT_PARSER_SRC

        ../smartbody-lib/src/external/parser/Bchart.cpp
        ../smartbody-lib/src/external/parser/BchartSm.cpp
        ../smartbody-lib/src/external/parser/Bst.cpp
        ../smartbody-lib/src/external/parser/ChartBase.cpp
        ../smartbody-lib/src/external/parser/ClassRule.cpp
        ../smartbody-lib/src/external/parser/CntxArray.cpp
        ../smartbody-lib/src/external/parser/CombineBests.cpp
        ../smartbody-lib/src/external/parser/ECArgs.cpp
        ../smartbody-lib/src/external/parser/Edge.cpp
        ../smartbody-lib/src/external/parser/EdgeHeap.cpp
        ../smartbody-lib/src/external/parser/edgeSubFns.cpp
        ../smartbody-lib/src/external/parser/EgsFromTree.cpp
        ../smartbody-lib/src/external/parser/ewDciTokStrm.cpp
        ../smartbody-lib/src/external/parser/extraMain.cpp
        ../smartbody-lib/src/external/parser/FBinaryArray.cpp
        ../smartbody-lib/src/external/parser/Feat.cpp
        ../smartbody-lib/src/external/parser/Feature.cpp
        ../smartbody-lib/src/external/parser/FeatureTree.cpp
        ../smartbody-lib/src/external/parser/fhSubFns.cpp
        ../smartbody-lib/src/external/parser/Field.cpp
        ../smartbody-lib/src/external/parser/FullHist.cpp
        ../smartbody-lib/src/external/parser/GotIter.cpp
        ../smartbody-lib/src/external/parser/headFinder.cpp
        ../smartbody-lib/src/external/parser/headFinderCh.cpp
        ../smartbody-lib/src/external/parser/InputTree.cpp
        ../smartbody-lib/src/external/parser/Item.cpp
        ../smartbody-lib/src/external/parser/Link.cpp
        ../smartbody-lib/src/external/parser/MeChart.cpp
        ../smartbody-lib/src/external/parser/Params.cpp
        ../smartbody-lib/src/external/parser/ParseStats.cpp
        ../smartbody-lib/src/external/parser/SentRep.cpp
        ../smartbody-lib/src/external/parser/Term.cpp
        ../smartbody-lib/src/external/parser/TimeIt.cpp
        ../smartbody-lib/src/external/parser/UnitRules.cpp
        ../smartbody-lib/src/external/parser/utils.cpp
        ../smartbody-lib/src/external/parser/ValHeap.cpp
)


set( SMARTBODY_LIB_EXT_SOIL_SRC

        ../smartbody-lib/src/external/recast/Recast.cpp
        ../smartbody-lib/src/external/recast/RecastAlloc.cpp
        ../smartbody-lib/src/external/recast/RecastArea.cpp
        ../smartbody-lib/src/external/recast/RecastContour.cpp
        ../smartbody-lib/src/external/recast/RecastFilter.cpp
        ../smartbody-lib/src/external/recast/RecastLayers.cpp
        ../smartbody-lib/src/external/recast/RecastMesh.cpp
        ../smartbody-lib/src/external/recast/RecastMeshDetail.cpp
        ../smartbody-lib/src/external/recast/RecastRasterization.cpp
        ../smartbody-lib/src/external/recast/RecastRegion.cpp
        ../smartbody-lib/src/external/recast/DetourAlloc.cpp
        ../smartbody-lib/src/external/recast/DetourCommon.cpp
        ../smartbody-lib/src/external/recast/DetourNavMesh.cpp
        ../smartbody-lib/src/external/recast/DetourNavMeshBuilder.cpp
        ../smartbody-lib/src/external/recast/DetourNavMeshQuery.cpp
        ../smartbody-lib/src/external/recast/DetourNode.cpp
)

set( SMARTBODY_LIB_EXT_RECAST_SRC

        ../smartbody-lib/src/external/SOIL/image_DXT.c
        ../smartbody-lib/src/external/SOIL/image_helper.c
        ../smartbody-lib/src/external/SOIL/SOIL.c
        ../smartbody-lib/src/external/SOIL/stb_image_aug.c
#       ../smartbody-lib/src/external/SOIL/stbi_DDS_aug_c.h
)

set( SMARTBODY_LIB_EXT_PERLIN_SRC

        ../smartbody-lib/src/external/perlin/perlin.cpp
)

set( SMARTBODY_LIB_ME_SRC

        ../smartbody-lib/src/controllers/me_controller_context.cpp
        ../smartbody-lib/src/controllers/me_controller_context_proxy.cpp
        ../smartbody-lib/src/controllers/me_controller.cpp
        ../smartbody-lib/src/controllers/me_controller_tree_root.cpp
        ../smartbody-lib/src/controllers/me_ct_adshr_envelope.cpp
        ../smartbody-lib/src/controllers/me_ct_blend.cpp
        ../smartbody-lib/src/controllers/me_ct_channel_writer.cpp
        ../smartbody-lib/src/controllers/me_ct_container.cpp
        ../smartbody-lib/src/controllers/me_ct_curve_writer.cpp
        ../smartbody-lib/src/controllers/me_ct_interpolator.cpp
        ../smartbody-lib/src/controllers/me_ct_lifecycle_test.cpp
        ../smartbody-lib/src/controllers/me_ct_motion.cpp
        ../smartbody-lib/src/controllers/me_ct_periodic_replay.cpp
        ../smartbody-lib/src/controllers/me_ct_pose.cpp
        ../smartbody-lib/src/controllers/me_ct_scheduler2.cpp
        ../smartbody-lib/src/controllers/me_ct_time_shift_warp.cpp
        ../smartbody-lib/src/controllers/me_ct_unary.cpp
        ../smartbody-lib/src/controllers/me_default_prune_policy.cpp
        ../smartbody-lib/src/controllers/me_spline_1d.cpp
)

set( SMARTBODY_LIB_SK_SRC

        ../smartbody-lib/src/sk/sk_channel_array.cpp
        ../smartbody-lib/src/sk/sk_channel.cpp
        ../smartbody-lib/src/sk/sk_joint.cpp
        ../smartbody-lib/src/sk/sk_joint_euler.cpp
        ../smartbody-lib/src/sk/sk_joint_pos.cpp
        ../smartbody-lib/src/sk/sk_joint_quat.cpp
        ../smartbody-lib/src/sk/sk_joint_swing_twist.cpp
        ../smartbody-lib/src/sk/sk_motion.cpp
        ../smartbody-lib/src/sk/sk_motion_io.cpp
        ../smartbody-lib/src/sk/sk_posture.cpp
        ../smartbody-lib/src/sk/sk_scene.cpp
        ../smartbody-lib/src/sk/sk_skeleton.cpp
        ../smartbody-lib/src/sk/sk_skeleton_io.cpp
        ../smartbody-lib/src/sk/sk_vec_limits.cpp
)

set( SMARTBODY_LIB_SR_SRC

        ../smartbody-lib/src/sr/sr_alg.cpp
        ../smartbody-lib/src/sr/sr_array.cpp
        ../smartbody-lib/src/sr/sr_box.cpp
        ../smartbody-lib/src/sr/sr_buffer.cpp
        ../smartbody-lib/src/sr/sr_camera.cpp
        ../smartbody-lib/src/sr/sr_color.cpp
        ../smartbody-lib/src/sr/sr.cpp
        ../smartbody-lib/src/sr/sr_cylinder.cpp
        ../smartbody-lib/src/sr/sr_euler.cpp
        ../smartbody-lib/src/sr/sr_event.cpp
        ../smartbody-lib/src/sr/sr_geo2.cpp
        ../smartbody-lib/src/sr/sr_gl.cpp
        ../smartbody-lib/src/sr/sr_gl_render_funcs.cpp
        ../smartbody-lib/src/sr/sr_hash_table.cpp
        ../smartbody-lib/src/sr/sr_input.cpp
        ../smartbody-lib/src/sr/sr_light.cpp
        ../smartbody-lib/src/sr/sr_line.cpp
        ../smartbody-lib/src/sr/sr_lines.cpp
        ../smartbody-lib/src/sr/sr_mat.cpp
        ../smartbody-lib/src/sr/sr_material.cpp
        ../smartbody-lib/src/sr/sr_model.cpp
        ../smartbody-lib/src/sr/sr_model_export_iv.cpp
        ../smartbody-lib/src/sr/sr_model_import_obj.cpp
        ../smartbody-lib/src/sr/sr_output.cpp
        ../smartbody-lib/src/sr/sr_path_array.cpp
        ../smartbody-lib/src/sr/sr_plane.cpp
        ../smartbody-lib/src/sr/sr_points.cpp
        ../smartbody-lib/src/sr/sr_polygon.cpp
        ../smartbody-lib/src/sr/sr_polygons.cpp
        ../smartbody-lib/src/sr/sr_quat.cpp
        ../smartbody-lib/src/sr/sr_sa_bbox.cpp
        ../smartbody-lib/src/sr/sr_sa.cpp
        ../smartbody-lib/src/sr/sr_random.cpp
        ../smartbody-lib/src/sr/sr_sa_event.cpp
        ../smartbody-lib/src/sr/sr_sa_gl_render.cpp
        ../smartbody-lib/src/sr/sr_sa_render_mode.cpp
        ../smartbody-lib/src/sr/sr_shared_ptr.cpp
        ../smartbody-lib/src/sr/sr_sn.cpp
	../smartbody-lib/src/sr/sr_sn_editor.cpp
        ../smartbody-lib/src/sr/sr_sn_group.cpp
        ../smartbody-lib/src/sr/sr_sn_matrix.cpp
        ../smartbody-lib/src/sr/sr_sn_shape.cpp
        ../smartbody-lib/src/sr/sr_sn_colorsurf.cpp
        ../smartbody-lib/src/sr/sr_sphere.cpp
        ../smartbody-lib/src/sr/sr_spline.cpp
        ../smartbody-lib/src/sr/sr_string_array.cpp
        ../smartbody-lib/src/sr/sr_string.cpp
        ../smartbody-lib/src/sr/sr_timer.cpp
        ../smartbody-lib/src/sr/sr_trackball.cpp
        ../smartbody-lib/src/sr/sr_tree.cpp
        ../smartbody-lib/src/sr/sr_triangle.cpp
        ../smartbody-lib/src/sr/sr_vec2.cpp
        ../smartbody-lib/src/sr/sr_vec.cpp
        ../smartbody-lib/src/sr/sr_viewer.cpp
        ../smartbody-lib/src/sr/planner/sk_blend_cfg.cpp
        ../smartbody-lib/src/sr/planner/sk_pos_planner.cpp
        ../smartbody-lib/src/sr/planner/sk_blend_cfg_manager.cpp
        ../smartbody-lib/src/sr/planner/sr_cfg_planner.cpp
        ../smartbody-lib/src/sr/planner/sk_blend_planner.cpp
        ../smartbody-lib/src/sr/planner/sr_cfg_manager.cpp
        ../smartbody-lib/src/sr/planner/sk_pos_cfg.cpp
        ../smartbody-lib/src/sr/planner/sr_cfg_planner_lazy.cpp
        ../smartbody-lib/src/sr/planner/sr_cfg_path.cpp
        ../smartbody-lib/src/sr/planner/sr_cfg_tree.cpp
        ../smartbody-lib/src/sr/planner/sk_pos_cfg_manager.cpp
)

set( SMARTBODY_LIB_SBM_SRC

        ../smartbody-lib/src/sbm/BMLDefs.cpp
        ../smartbody-lib/src/bml/behavior_scheduler_constant_speed.cpp
        ../smartbody-lib/src/bml/behavior_scheduler.cpp
        ../smartbody-lib/src/bml/behavior_scheduler_fixed.cpp
        ../smartbody-lib/src/bml/behavior_span.cpp
        ../smartbody-lib/src/bml/bml_animation.cpp
        ../smartbody-lib/src/bml/bml_gesture.cpp
        ../smartbody-lib/src/bml/bml_bodyreach.cpp
        ../smartbody-lib/src/bml/bml_constraint.cpp
        ../smartbody-lib/src/bml/bml.cpp
        ../smartbody-lib/src/bml/bml_event.cpp
        ../smartbody-lib/src/bml/bml_face.cpp
        ../smartbody-lib/src/bml/bml_gaze.cpp
        ../smartbody-lib/src/bml/bml_general_param.cpp
        ../smartbody-lib/src/bml/bml_grab.cpp
        ../smartbody-lib/src/bml/bml_interrupt.cpp
        ../smartbody-lib/src/bml/bml_locomotion.cpp
        ../smartbody-lib/src/bml/bml_processor.cpp
        ../smartbody-lib/src/bml/bml_quickdraw.cpp
        ../smartbody-lib/src/bml/bml_reach.cpp
        ../smartbody-lib/src/bml/bml_speech.cpp
        ../smartbody-lib/src/bml/bml_sync_point.cpp
        ../smartbody-lib/src/bml/bml_target.cpp
        ../smartbody-lib/src/bml/bml_saccade.cpp
        ../smartbody-lib/src/bml/bml_noise.cpp
	../smartbody-lib/src/bml/bml_states.cpp
        ../smartbody-lib/src/sbm/GenericViewer.cpp
        ../smartbody-lib/src/sbm/gwiz_cmdl.cpp
        ../smartbody-lib/src/sbm/gwiz_math.cpp
        ../smartbody-lib/src/sbm/gwiz_spline.cpp
        ../smartbody-lib/src/sbm/Heightfield.cpp
        ../smartbody-lib/src/sbm/lin_win.cpp
        ../smartbody-lib/src/sbm/mcontrol_callbacks.cpp
        ../smartbody-lib/src/sbm/action_unit.cpp

        ../smartbody-lib/src/controllers/me_ct_basic_locomotion.cpp
        ../smartbody-lib/src/controllers/me_ct_ccd_IK.cpp
        ../smartbody-lib/src/controllers/me_ct_constraint.cpp
        ../smartbody-lib/src/controllers/me_ct_data_driven_reach.cpp
        ../smartbody-lib/src/controllers/me_ct_data_interpolation.cpp
        ../smartbody-lib/src/controllers/me_ct_example_body_reach.cpp
        ../smartbody-lib/src/controllers/me_ct_examples.cpp
        ../smartbody-lib/src/controllers/me_ct_eyelid.cpp
        ../smartbody-lib/src/controllers/me_ct_face.cpp
        ../smartbody-lib/src/controllers/me_ct_gaze_alg.cpp
        ../smartbody-lib/src/controllers/me_ct_gaze.cpp
        ../smartbody-lib/src/controllers/me_ct_gaze_joint.cpp
        ../smartbody-lib/src/controllers/me_ct_gaze_keymap.cpp
        ../smartbody-lib/src/controllers/me_ct_gaze_target.cpp
        ../smartbody-lib/src/controllers/me_ct_IK.cpp
        ../smartbody-lib/src/controllers/me_ct_IK_scenario.cpp
        ../smartbody-lib/src/controllers/me_ct_jacobian_IK.cpp
        ../smartbody-lib/src/controllers/me_ct_limb.cpp
        ../smartbody-lib/src/controllers/me_ct_locomotion_func.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_example.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_parameter.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_player.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_timewarp.cpp
        ../smartbody-lib/src/controllers/me_ct_param_animation.cpp
        ../smartbody-lib/src/controllers/me_ct_param_animation_utilities.cpp
        ../smartbody-lib/src/controllers/me_ct_quick_draw.cpp
        ../smartbody-lib/src/controllers/me_ct_reach.cpp
        ../smartbody-lib/src/controllers/me_ct_hand.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_profile.cpp
        ../smartbody-lib/src/controllers/me_ct_barycentric_interpolation.cpp
        ../smartbody-lib/src/controllers/me_ct_inverse_interpolation.cpp
        ../smartbody-lib/src/controllers/me_ct_reach_IK.cpp
        ../smartbody-lib/src/controllers/me_ct_simple_gaze.cpp
        ../smartbody-lib/src/controllers/me_ct_tether.cpp
        ../smartbody-lib/src/controllers/me_ct_ublas.cpp
        ../smartbody-lib/src/controllers/me_ct_breathing.cpp
        ../smartbody-lib/src/controllers/me_ct_breathing_interface.cpp
        ../smartbody-lib/src/controllers/me_ct_physics_controller.cpp
        ../smartbody-lib/src/controllers/me_ct_noise_controller.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_recorder.cpp

        ../smartbody-lib/src/controllers/MeCtBlendEngine.cpp
        ../smartbody-lib/src/controllers/me_ct_motion_blend.cpp
        ../smartbody-lib/src/controllers/MotionAnalysis.cpp

        ../smartbody-lib/src/sbm/ParserBVH.cpp
        ../smartbody-lib/src/sbm/ParserASFAMC.cpp
        ../smartbody-lib/src/sbm/ParserFBX.cpp
        ../smartbody-lib/src/sbm/ParserOgre.cpp
        ../smartbody-lib/src/sbm/ParserOpenCOLLADA.cpp
        ../smartbody-lib/src/sbm/remote_speech.cpp
        ../smartbody-lib/src/sbm/sbm_audio.cpp
        ../smartbody-lib/src/controllers/MeCtBodyReachState.cpp
        ../smartbody-lib/src/controllers/MeCtReachEngine.cpp
        ../smartbody-lib/src/controllers/me_ct_saccade.cpp
        ../smartbody-lib/src/sbm/local_speech.cpp
## UNUSED:
##      ../smartbody-lib/src/sbm/sbm_../smartbody-lib/src/bml.cpp
        ../smartbody-lib/src/sbm/sbm_deformable_mesh.cpp
        ../smartbody-lib/src/sbm/sbm_speech_audiofile.cpp
        ../smartbody-lib/src/sbm/sbm_speech.cpp
        ../smartbody-lib/src/sbm/sbm_speech_impl_skeleton.cpp
        ../smartbody-lib/src/sbm/sbm_test_cmds.cpp
        ../smartbody-lib/src/sbm/sr_arg_buff.cpp
        ../smartbody-lib/src/sbm/sr_cmd_line.cpp
        ../smartbody-lib/src/sbm/sr_cmd_seq.cpp
        ../smartbody-lib/src/sbm/sr_hash_map.cpp
        ../smartbody-lib/src/sbm/sr_linear_curve.cpp
        ../smartbody-lib/src/sbm/sr_spline_curve.cpp
        ../smartbody-lib/src/sbm/sr_synch_points.cpp
        ../smartbody-lib/src/sbm/text_speech.cpp
        ../smartbody-lib/src/sbm/time_profiler.cpp
        ../smartbody-lib/src/sbm/time_regulator.cpp
        ../smartbody-lib/src/sbm/xercesc_utils.cpp


        ../smartbody-lib/src/sbm/ODEPhysicsSim.cpp

        ../smartbody-lib/src/sbm/GPU/SbmTexture.cpp
        ../smartbody-lib/src/sbm/GPU/SbmDeformableMeshGPU.cpp
        ../smartbody-lib/src/sbm/GPU/TBOData.cpp
        ../smartbody-lib/src/sbm/GPU/VBOData.cpp
        ../smartbody-lib/src/sbm/GPU/SbmShader.cpp
        ../smartbody-lib/src/sbm/PPRAISteeringAgent.cpp
        ../smartbody-lib/src/sbm/SteerSuiteEnginerDriver.cpp
        ../smartbody-lib/src/sbm/sr_path_list.cpp

        ../smartbody-lib/src/sb/sbm_character.cpp
        ../smartbody-lib/src/sb/sbm_pawn.cpp

        ../smartbody-lib/src/sb/SBAttribute.cpp
        ../smartbody-lib/src/sb/SBAttributeManager.cpp
        ../smartbody-lib/src/sb/SBObject.cpp
        ../smartbody-lib/src/sb/SBObserver.cpp
        ../smartbody-lib/src/sb/SBSubject.cpp
        ../smartbody-lib/src/sb/DefaultAttributeTable.cpp

        ../smartbody-lib/src/sb/SBScene.cpp
        ../smartbody-lib/src/sbm/KinectProcessor.cpp
        ../smartbody-lib/src/controllers/me_ct_data_receiver.cpp

        ../smartbody-lib/src/sb/SBScript.cpp
        ../smartbody-lib/src/sb/SBCharacter.cpp
        ../smartbody-lib/src/sb/SBPawn.cpp
        ../smartbody-lib/src/sb/SBJoint.cpp
        ../smartbody-lib/src/sb/SBSkeleton.cpp
        ../smartbody-lib/src/sb/SBController.cpp
        ../smartbody-lib/src/sb/SBSimulationManager.cpp
        ../smartbody-lib/src/sb/SBBmlProcessor.cpp
        ../smartbody-lib/src/sb/SBAnimationState.cpp
        ../smartbody-lib/src/sb/SBAnimationTransition.cpp
        ../smartbody-lib/src/sb/SBAnimationTransitionRule.cpp
        ../smartbody-lib/src/sb/SBAnimationStateManager.cpp
        ../smartbody-lib/src/sb/SBSteerManager.cpp
        ../smartbody-lib/src/sb/SBSteerAgent.cpp
        ../smartbody-lib/src/sb/SBReachManager.cpp
        ../smartbody-lib/src/sb/SBReach.cpp
        ../smartbody-lib/src/sb/SBGestureMapManager.cpp
        ../smartbody-lib/src/sb/SBGestureMap.cpp
        ../smartbody-lib/src/sb/SBServiceManager.cpp
        ../smartbody-lib/src/sb/SBService.cpp
        ../smartbody-lib/src/sb/SBMotion.cpp
        ../smartbody-lib/src/sb/SBFaceDefinition.cpp
        ../smartbody-lib/src/sb/SBPhysicsManager.cpp
        ../smartbody-lib/src/sb/SBBoneBusManager.cpp
        ../smartbody-lib/src/sb/SBCollisionManager.cpp
        ../smartbody-lib/src/sb/SBParser.cpp
        ../smartbody-lib/src/sb/SBParseNode.cpp
        ../smartbody-lib/src/sb/SBJointMap.cpp
        ../smartbody-lib/src/sb/SBJointMapManager.cpp
        ../smartbody-lib/src/sb/SBPhoneme.cpp
        ../smartbody-lib/src/sb/SBPhonemeManager.cpp
        ../smartbody-lib/src/sb/SBBehaviorSet.cpp
        ../smartbody-lib/src/sb/SBBehaviorSetManager.cpp
        ../smartbody-lib/src/sb/SBMotionBlendBase.cpp
        ../smartbody-lib/src/sb/nvbg.cpp
        ../smartbody-lib/src/sb/MiniBrain.cpp
        ../smartbody-lib/src/sb/SBColObject.cpp
        ../smartbody-lib/src/sb/SBPhysicsSim.cpp
        ../smartbody-lib/src/sb/SBEvent.cpp
        ../smartbody-lib/src/sb/PABlend.cpp
        ../smartbody-lib/src/sb/SBRetarget.cpp
        ../smartbody-lib/src/sb/SBRetargetManager.cpp
        ../smartbody-lib/src/sb/SBNavigationMesh.cpp
        ../smartbody-lib/src/sb/SBAsset.cpp
        ../smartbody-lib/src/sb/SBAssetManager.cpp
        ../smartbody-lib/src/sb/SBAsset.cpp
        ../smartbody-lib/src/sb/SBSpeechManager.cpp
        ../smartbody-lib/src/sb/SBVHMsgManager.cpp
        ../smartbody-lib/src/sb/SBCommandManager.cpp
        ../smartbody-lib/src/sb/SBWSPManager.cpp

        ../smartbody-lib/src/sb/SBPython.cpp
        ../smartbody-lib/src/sb/SBPythonAnimation.cpp
        ../smartbody-lib/src/sb/SBPythonAttribute.cpp
        ../smartbody-lib/src/sb/SBPythonCharacter.cpp
        ../smartbody-lib/src/sb/SBPythonMath.cpp
        ../smartbody-lib/src/sb/SBPythonMotion.cpp
        ../smartbody-lib/src/sb/SBPythonScene.cpp
        ../smartbody-lib/src/sb/SBPythonSimulation.cpp
        ../smartbody-lib/src/sb/SBPythonSkeleton.cpp
        ../smartbody-lib/src/sb/SBPythonSystem.cpp
        ../smartbody-lib/src/sb/SBPythonClass.cpp

        ../smartbody-lib/src/sb/SBBehavior.cpp
        ../smartbody-lib/src/sb/SBDebuggerServer.cpp
        ../smartbody-lib/src/sb/SBDebuggerClient.cpp
        ../smartbody-lib/src/sb/SBDebuggerUtility.cpp
        ../smartbody-lib/src/sbm/SteerPath.cpp
        ../smartbody-lib/src/sbm/MiscCommands.cpp
)

add_library(
	SmartBody
	SHARED 

	${SMARTBODY_LIB_EXT_ANN_SRC}

        ${SMARTBODY_LIB_EXT_SOIL_SRC}

        ${SMARTBODY_LIB_EXT_PARSER_SRC}

        ${SMARTBODY_LIB_EXT_RECAST_SRC}

        ${SMARTBODY_LIB_EXT_PERLIN_SRC}

        ${SMARTBODY_LIB_ME_SRC}

        ${SMARTBODY_LIB_SK_SRC}

        ${SMARTBODY_LIB_SR_SRC}

        ${SMARTBODY_LIB_SBM_SRC}

	
	../smartbody-dll/smartbody-dll.cpp
	../smartbody-dll/smartbody-c-dll.cpp
)


target_link_libraries( SmartBody

#	tcmalloc
#	profiler
	bonebus
	vhmsg
	vhcl
	wsp
	xerces-c
	boost_filesystem
	boost_system
	boost_regex
	boost_python
	blas
	lapack
	${OPENGL_LIBRARIES}
	activemq-cpp
	steerlib
	pprAI
	ode
	python2.7
	ncurses
	${MORE_LIBS}
)

set( EXEC_LIST

       SmartBody
)

set( APP_LIBRARY_SOURCE_PATHS

${PROJECT_SOURCE_DIR}/core/smartbody/sbm/bin
)       

set_target_properties(
	${EXEC_LIST}
	PROPERTIES
	## link with install tree:
	INSTALL_RPATH                   "${APP_LIBRARY_SOURCE_PATHS}"
	BUILD_WITH_INSTALL_RPATH        TRUE
)

install(

        TARGETS ${EXEC_LIST}
	DESTINATION core/smartbody/sbm/bin
)

