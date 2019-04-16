#include "oneflow/core/operator/yolo_box_diff_op.h"

namespace oneflow {

void YoloBoxDiffOp::InitFromOpConf() {
  CHECK(op_conf().has_yolo_box_diff_conf());
  // Enroll input
  EnrollInputBn("bbox", true);
  EnrollInputBn("gt_boxes", false);
  EnrollInputBn("gt_labels", false);
  // Enroll output
  EnrollOutputBn("bbox_loc_diff", true);
  EnrollOutputBn("pos_inds", false);
  EnrollOutputBn("pos_cls_label", false);
  EnrollOutputBn("neg_inds", false);
  EnrollOutputBn("valid_num", false);
  // data tmp
  EnrollDataTmpBn("pred_bbox");
  EnrollDataTmpBn("anchor_boxes_size_tmp");
  EnrollDataTmpBn("box_mask_tmp");
  EnrollDataTmpBn("overlaps");
  EnrollDataTmpBn("max_overlaps");
  EnrollDataTmpBn("max_overlaps_gt_indices");
}

const PbMessage& YoloBoxDiffOp::GetCustomizedConf() const { return op_conf().yolo_box_diff_conf(); }

void YoloBoxDiffOp::InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                                   const ParallelContext* parallel_ctx) const {
  // input: bbox : (n, r, 4)  r = h*w*3
  const BlobDesc* bbox_blob_desc = GetBlobDesc4BnInOp("bbox");
  // input: gt_boxes (n, g, 4) T
  const BlobDesc* gt_boxes_blob_desc = GetBlobDesc4BnInOp("gt_boxes");
  // input: gt_labels (n, g) int32_t
  const BlobDesc* gt_labels_blob_desc = GetBlobDesc4BnInOp("gt_labels");
  const int64_t num_images = bbox_blob_desc->shape().At(0);
  CHECK_EQ(num_images, gt_boxes_blob_desc->shape().At(0));
  CHECK_EQ(num_images, gt_labels_blob_desc->shape().At(0));
  const int64_t num_boxes = bbox_blob_desc->shape().At(1);
  const int64_t max_num_gt_boxes = gt_boxes_blob_desc->shape().At(1);
  CHECK_EQ(max_num_gt_boxes, gt_labels_blob_desc->shape().At(1));
  CHECK_EQ(bbox_blob_desc->data_type(), gt_boxes_blob_desc->data_type());
  CHECK(gt_boxes_blob_desc->has_dim1_valid_num_field());
  CHECK(gt_labels_blob_desc->has_dim1_valid_num_field());

  // output: bbox_loc_diff (n, r, 4)
  BlobDesc* bbox_loc_diff_blob_desc = GetBlobDesc4BnInOp("bbox_loc_diff");
  bbox_loc_diff_blob_desc->mut_shape() = Shape({num_images, num_boxes, 4});
  bbox_loc_diff_blob_desc->set_data_type(bbox_blob_desc->data_type());
  // output: pos_cls_label (n, r)
  BlobDesc* pos_cls_label_blob_desc = GetBlobDesc4BnInOp("pos_cls_label");
  pos_cls_label_blob_desc->mut_shape() = Shape({num_images, num_boxes});
  pos_cls_label_blob_desc->set_data_type(DataType::kInt32);
  // output: pos_inds (n, r) dynamic
  BlobDesc* pos_inds_blob_desc = GetBlobDesc4BnInOp("pos_inds");
  pos_inds_blob_desc->mut_shape() = Shape({num_images, num_boxes});
  pos_inds_blob_desc->set_data_type(DataType::kInt32);
  pos_inds_blob_desc->set_has_dim1_valid_num_field(true);
  // output: neg_inds (n, r) dynamic
  BlobDesc* neg_inds_blob_desc = GetBlobDesc4BnInOp("neg_inds");
  neg_inds_blob_desc->mut_shape() = Shape({num_images, num_boxes});
  neg_inds_blob_desc->set_data_type(DataType::kInt32);
  neg_inds_blob_desc->set_has_dim1_valid_num_field(true);
  // output: valid_num
  BlobDesc* valid_num_blob_desc = GetBlobDesc4BnInOp("valid_num");
  valid_num_blob_desc->mut_shape() = Shape({num_images, 2});
  valid_num_blob_desc->set_data_type(DataType::kInt32);

  // tmp: pred_bbox (r, 4) T
  *GetBlobDesc4BnInOp("pred_bbox") = *bbox_blob_desc;
  BlobDesc* pred_bbox_blob_desc = GetBlobDesc4BnInOp("pred_bbox");
  pred_bbox_blob_desc->mut_shape() = Shape({num_boxes, 4});
  pred_bbox_blob_desc->set_data_type(bbox_blob_desc->data_type());

  // tmp: anchor_boxes_size (2 * 9) int32_t
  const int32_t anchor_boxes_size_num = 2 * op_conf().yolo_box_diff_conf().anchor_boxes_size_size();
  BlobDesc* anchor_boxes_size_tmp_blob_desc = GetBlobDesc4BnInOp("anchor_boxes_size_tmp");
  anchor_boxes_size_tmp_blob_desc->mut_shape() = Shape({anchor_boxes_size_num});
  anchor_boxes_size_tmp_blob_desc->set_data_type(DataType::kInt32);

  // tmp: box_mask (3) int32_t
  const int32_t box_mask_num = op_conf().yolo_box_diff_conf().box_mask_size();
  BlobDesc* box_mask_tmp_blob_desc = GetBlobDesc4BnInOp("box_mask_tmp");
  box_mask_tmp_blob_desc->mut_shape() = Shape({box_mask_num});
  box_mask_tmp_blob_desc->set_data_type(DataType::kInt32);

  // tmp: overlaps (r, gt_max_num) float
  BlobDesc* overlaps_blob_desc = GetBlobDesc4BnInOp("overlaps");
  overlaps_blob_desc->mut_shape() = Shape({num_boxes, max_num_gt_boxes});
  overlaps_blob_desc->set_data_type(DataType::kFloat);

  // tmp: max_overlaps (r) float
  BlobDesc* max_overlaps_blob_desc = GetBlobDesc4BnInOp("max_overlaps");
  max_overlaps_blob_desc->mut_shape() = Shape({num_boxes});
  max_overlaps_blob_desc->set_data_type(DataType::kFloat);

  // tmp: max_overlaps_gt_indices (r) int32_t
  BlobDesc* max_overlaps_gt_indices_blob_desc = GetBlobDesc4BnInOp("max_overlaps_gt_indices");
  max_overlaps_gt_indices_blob_desc->mut_shape() = Shape({num_boxes});
  max_overlaps_gt_indices_blob_desc->set_data_type(DataType::kInt32);
}

// REGISTER_OP(OperatorConf::kYoloBoxDiffConf, YoloBoxDiffOp);
REGISTER_OP(OperatorConf::kYoloBoxDiffConf, YoloBoxDiffOp);
}  // namespace oneflow
