#pragma once

struct ModelKeyframeData // Types.h :: asKeyframeData에 대응되는 구조체
{
	float Time;

	Vector3 Scale;
	Quaternion Rotation;
	Vector3 Translation;
};

struct ModelKeyframe // Types.h :: asKeyframe에 대응되는 구조체
{
	wstring BoneName;
	vector<ModelKeyframeData> Transforms;
};

class ModelClip
{
public:
	friend class Model;
	friend class ModelPBR;
private:
	ModelClip();
	~ModelClip();

public:
	float Duration() { return duration; }
	float FrameRate() { return frameRate; }
	UINT FrameCount() { return frameCount; }

	ModelKeyframe* Keyframe(wstring name);

private:
	wstring name;

	float duration;
	float frameRate;
	UINT frameCount;

	unordered_map<wstring, ModelKeyframe *> keyframeMap;
};