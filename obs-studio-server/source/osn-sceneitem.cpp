#include "osn-sceneitem.hpp"
#include "osn-source.hpp"
#include <error.hpp>

void osn::SceneItem::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("SceneItem");
	cls->register_function(std::make_shared<ipc::function>("GetSource", std::vector<ipc::type>{ipc::type::UInt64}, GetSource));
	cls->register_function(std::make_shared<ipc::function>("GetScene", std::vector<ipc::type>{ipc::type::UInt64}, GetScene));
	cls->register_function(std::make_shared<ipc::function>("Remove", std::vector<ipc::type>{ipc::type::UInt64}, Remove));
	cls->register_function(std::make_shared<ipc::function>("IsVisible", std::vector<ipc::type>{ipc::type::UInt64}, IsVisible));
	cls->register_function(std::make_shared<ipc::function>("SetVisible", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetVisible));
	cls->register_function(std::make_shared<ipc::function>("IsSelected", std::vector<ipc::type>{ipc::type::UInt64}, IsSelected));
	cls->register_function(std::make_shared<ipc::function>("SetSelected", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetSelected));
	cls->register_function(std::make_shared<ipc::function>("GetPosition", std::vector<ipc::type>{ipc::type::UInt64}, GetPosition));
	cls->register_function(std::make_shared<ipc::function>("SetPosition", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float, ipc::type::Float}, SetPosition));
	cls->register_function(std::make_shared<ipc::function>("GetRotation", std::vector<ipc::type>{ipc::type::UInt64}, GetRotation));
	cls->register_function(std::make_shared<ipc::function>("SetRotation", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetRotation));
	cls->register_function(std::make_shared<ipc::function>("GetScale", std::vector<ipc::type>{ipc::type::UInt64}, GetScale));
	cls->register_function(std::make_shared<ipc::function>("SetScale", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float, ipc::type::Float}, SetScale));
	cls->register_function(std::make_shared<ipc::function>("GetScaleFilter", std::vector<ipc::type>{ipc::type::UInt64}, GetScaleFilter));
	cls->register_function(std::make_shared<ipc::function>("SetScaleFilter", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetScaleFilter));
	cls->register_function(std::make_shared<ipc::function>("GetAlignment", std::vector<ipc::type>{ipc::type::UInt64}, GetAlignment));
	cls->register_function(std::make_shared<ipc::function>("SetAlignment", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetAlignment));
	cls->register_function(std::make_shared<ipc::function>("GetBounds", std::vector<ipc::type>{ipc::type::UInt64}, GetBounds));
	cls->register_function(std::make_shared<ipc::function>("SetBounds", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float, ipc::type::Float}, SetBounds));
	cls->register_function(std::make_shared<ipc::function>("GetBoundsAlignment", std::vector<ipc::type>{ipc::type::UInt64}, GetBoundsAlignment));
	cls->register_function(std::make_shared<ipc::function>("SetBoundsAlignment", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetBoundsAlignment));
	cls->register_function(std::make_shared<ipc::function>("GetBoundsType", std::vector<ipc::type>{ipc::type::UInt64}, GetBoundsType));
	cls->register_function(std::make_shared<ipc::function>("SetBoundsType", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetBoundsType));
	cls->register_function(std::make_shared<ipc::function>("GetCrop", std::vector<ipc::type>{ipc::type::UInt64}, GetCrop));
	cls->register_function(std::make_shared<ipc::function>("SetCrop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32, ipc::type::Int32, ipc::type::Int32, ipc::type::Int32}, SetCrop));
	cls->register_function(std::make_shared<ipc::function>("GetId", std::vector<ipc::type>{ipc::type::UInt64}, GetId));
	cls->register_function(std::make_shared<ipc::function>("MoveUp", std::vector<ipc::type>{ipc::type::UInt64}, MoveUp));
	cls->register_function(std::make_shared<ipc::function>("MoveDown", std::vector<ipc::type>{ipc::type::UInt64}, MoveDown));
	cls->register_function(std::make_shared<ipc::function>("MoveTop", std::vector<ipc::type>{ipc::type::UInt64}, MoveTop));
	cls->register_function(std::make_shared<ipc::function>("MoveBottom", std::vector<ipc::type>{ipc::type::UInt64}, MoveBottom));
	cls->register_function(std::make_shared<ipc::function>("Move", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, Move));
	cls->register_function(std::make_shared<ipc::function>("DeferUpdateBegin", std::vector<ipc::type>{ipc::type::UInt64}, DeferUpdateBegin));
	cls->register_function(std::make_shared<ipc::function>("DeferUpdateEnd", std::vector<ipc::type>{ipc::type::UInt64}, DeferUpdateEnd));
	srv.register_collection(cls);
}

void osn::SceneItem::GetSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_source_t* source = obs_sceneitem_get_source(item);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Item does not contain a source."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Get(source);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
}

void osn::SceneItem::GetScene(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_sceneitem_get_scene(item);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Item does not belong to a scene."));
		return;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Scene is invalid."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Get(source);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
}

void osn::SceneItem::Remove(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	osn::SceneItem::Manager::GetInstance().free(args[0].value_union.ui64);
	obs_sceneitem_release(item);
	obs_sceneitem_remove(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::IsVisible(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_visible(item)));
}

void osn::SceneItem::SetVisible(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_visible(item, !!args[1].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_visible(item)));
}

void osn::SceneItem::IsSelected(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_selected(item)));
}

void osn::SceneItem::SetSelected(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_select(item, !!args[1].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_selected(item)));
}

void osn::SceneItem::GetPosition(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(pos.x));
	rval.push_back(ipc::value(pos.y));
}

void osn::SceneItem::SetPosition(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 pos;
	pos.x = args[1].value_union.fp32;
	pos.y = args[2].value_union.fp32;

	obs_sceneitem_set_pos(item, &pos);
	obs_sceneitem_get_pos(item, &pos);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(pos.x));
	rval.push_back(ipc::value(pos.y));
}

void osn::SceneItem::GetRotation(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_get_rot(item)));
}

void osn::SceneItem::SetRotation(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_rot(item, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_get_rot(item)));
}

void osn::SceneItem::GetScale(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(scale.x));
	rval.push_back(ipc::value(scale.y));
}

void osn::SceneItem::SetScale(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 scale;
	scale.x = args[1].value_union.fp32;
	scale.y = args[2].value_union.fp32;

	obs_sceneitem_set_scale(item, &scale);
	obs_sceneitem_get_scale(item, &scale);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(scale.x));
	rval.push_back(ipc::value(scale.y));
}

void osn::SceneItem::GetScaleFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_scale_type type = obs_sceneitem_get_scale_filter(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(type));
}

void osn::SceneItem::SetScaleFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_scale_filter(item, (obs_scale_type)args[1].value_union.i32);
	obs_scale_type type = obs_sceneitem_get_scale_filter(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(type));
}

void osn::SceneItem::GetAlignment(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	uint32_t align = obs_sceneitem_get_alignment(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(align));
}

void osn::SceneItem::SetAlignment(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_alignment(item, args[1].value_union.ui32);
	uint32_t align = obs_sceneitem_get_alignment(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(align));
}

void osn::SceneItem::GetBounds(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 bounds;
	obs_sceneitem_get_bounds(item, &bounds);
	rval.push_back(ipc::value(bounds.x));
	rval.push_back(ipc::value(bounds.y));
}

void osn::SceneItem::SetBounds(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	vec2 bounds;
	bounds.x = args[1].value_union.fp32;
	bounds.y = args[2].value_union.fp32;

	obs_sceneitem_get_bounds(item, &bounds);
	rval.push_back(ipc::value(bounds.x));
	rval.push_back(ipc::value(bounds.y));
}

void osn::SceneItem::GetBoundsAlignment(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	uint32_t align = obs_sceneitem_get_bounds_alignment(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(align));
}

void osn::SceneItem::SetBoundsAlignment(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_bounds_alignment(item, args[1].value_union.ui32);
	uint32_t align = obs_sceneitem_get_bounds_alignment(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(align));
}

void osn::SceneItem::GetBoundsType(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_bounds_type bounds = obs_sceneitem_get_bounds_type(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(bounds));
}

void osn::SceneItem::SetBoundsType(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_bounds_type(item, (obs_bounds_type)args[1].value_union.i32);
	obs_bounds_type bounds = obs_sceneitem_get_bounds_type(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(bounds));
}

void osn::SceneItem::GetCrop(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(item, &crop);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(crop.left));
	rval.push_back(ipc::value(crop.top));
	rval.push_back(ipc::value(crop.right));
	rval.push_back(ipc::value(crop.bottom));
}

void osn::SceneItem::SetCrop(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_crop crop;
	crop.left = args[1].value_union.i32;
	crop.top = args[2].value_union.i32;
	crop.right = args[3].value_union.i32;
	crop.bottom = args[4].value_union.i32;

	obs_sceneitem_set_crop(item, &crop);
	obs_sceneitem_get_crop(item, &crop);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(crop.left));
	rval.push_back(ipc::value(crop.top));
	rval.push_back(ipc::value(crop.right));
	rval.push_back(ipc::value(crop.bottom));
}

void osn::SceneItem::GetId(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_sceneitem_get_id(item)));
}

void osn::SceneItem::MoveUp(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_UP);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::MoveDown(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_DOWN);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::MoveTop(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::MoveBottom(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_BOTTOM);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::Move(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_set_order_position(item, args[1].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));	
}

void osn::SceneItem::DeferUpdateBegin(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_defer_update_begin(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void osn::SceneItem::DeferUpdateEnd(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_sceneitem_t* item = osn::SceneItem::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Item reference is not valid."));
		return;
	}

	obs_sceneitem_defer_update_end(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

osn::SceneItem::Manager& osn::SceneItem::Manager::GetInstance() {
	// Thread Safe since C++13 (Visual Studio 2015, GCC 4.3).
	static Manager instance;
	return instance;
}