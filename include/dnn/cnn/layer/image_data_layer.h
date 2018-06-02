#ifdef USE_OPENCV


#include "caffe/blob.hpp"
#include "caffe/data_transformer.hpp"
#include "caffe/internal_thread.hpp"
#include "caffe/layer.hpp"
#include "base_data_layer.hpp"




/**
 * @brief Provides data to the Net from image files.
 *
 * TODO(dox): thorough documentation for Forward and proto params.
 */

class ImageDataLayer : public BasePrefetchingDataLayer {
 public:
  explicit ImageDataLayer(const LayerParameter& param)
      : BasePrefetchingDataLayer(param) {}
  virtual ~ImageDataLayer();
  virtual void DataLayerSetUp(const vector<Blob*>& bottom,
      const vector<Blob*>& top);

  virtual inline const char* type() const { return "ImageData"; }
  virtual inline int ExactNumBottomBlobs() const { return 0; }
  virtual inline int ExactNumTopBlobs() const { return -1; }
  virtual inline int MaxTopBlobs() const { return 3; }

 protected:
  shared_ptr<Caffe::RNG> prefetch_rng_;
  virtual void ShuffleImages();
  virtual void load_batch(Batch* batch);

  vector<std::pair<std::string, int> > lines_;
  int lines_id_;
  vector<int> num_samples_;
  vector class_weights_;
  bool balance_;
  vector<vector<std::pair<std::string, int> > > filename_by_class_;
  int class_id_;
};





#include "caffe/data_transformer.hpp"
#include "base_data_layer.hpp"
#include "image_data_layer.hpp"
#include "caffe/util/benchmark.hpp"
#include "caffe/util/io.hpp"

#include "caffe/util/rng.hpp"




ImageDataLayer::~ImageDataLayer() {
  this->StopInternalThread();
}


void ImageDataLayer::DataLayerSetUp(const vector<Blob*>& bottom,
      const vector<Blob*>& top) {
  const int new_height = this->layer_param_.image_data_param().new_height();
  const int new_width  = this->layer_param_.image_data_param().new_width();
  const bool is_color  = this->layer_param_.image_data_param().is_color();
  string root_folder = this->layer_param_.image_data_param().root_folder();
  balance_ = this->layer_param_.image_data_param().balance_class();

  CHECK((new_height == 0 && new_width == 0) ||
      (new_height > 0 && new_width > 0)) << "Current implementation requires "
      "new_height and new_width to be set at the same time.";
  // Read the file with filenames and labels
  const string& source = this->layer_param_.image_data_param().source();
  LOG(INFO) << "Opening file " << source;
  std::ifstream infile(source.c_str());
  string line;
  size_t pos;
  int label;
  lines_.clear();
  int max_label = 0;
  while (std::getline(infile, line)) {
    pos = line.find_last_of(' ');
    label = atoi(line.substr(pos + 1).c_str());
    lines_.push_back(std::make_pair(line.substr(0, pos), label));
    if (label > max_label) max_label = label;
  }

  CHECK(!lines_.empty()) << "File is empty";

  if (layer->o_size == 3) {
    num_samples_ = vector<int>(max_label + 1);
    class_weights_ = vector(max_label + 1);
    for (auto l : lines_) {
      num_samples_[l.second]++;
    }
    Dtype mean_sample_num = (Dtype)lines_.size() / (Dtype)(max_label + 1);
    Dtype min_weight = 9999, max_weight = 0;
    for (i = 0; i < num_samples_.size(); i++) {
      if (num_samples_[i] > 0) {
        class_weights_[i] = mean_sample_num / num_samples_[i];
        if (class_weights_[i] < min_weight) min_weight = class_weights_[i];
        if (class_weights_[i] > max_weight) max_weight = class_weights_[i];
      }
      else {
        class_weights_[i] = 1;
      }
    }
    LOG(INFO) << "label weight min:" << min_weight << " max:" << max_weight;
  }

  if (balance_) {
    num_samples_ = vector<int>(max_label + 1);
    filename_by_class_ = vector<vector<std::pair<std::string, int> > >(max_label + 1);
    for (auto l : lines_) {
      num_samples_[l.second]++;
      filename_by_class_[l.second].push_back(std::make_pair(l.first, 0));
    }
    class_id_ = 0;
  }

  if (this->layer_param_.image_data_param().shuffle()) {
    // randomly shuffle data
    LOG(INFO) << "Shuffling data";
    const unsigned int prefetch_rng_seed = blas_srng_rand();
    prefetch_rng_.reset(new Caffe::RNG(prefetch_rng_seed));
    ShuffleImages();
  } else {
    if (this->phase_ == TRAIN && Caffe::solver_rank() > 0 &&
        this->layer_param_.image_data_param().rand_skip() == 0) {
      LOG(WARNING) << "Shuffling or skipping recommended for multi-GPU";
    }
  }
  LOG(INFO) << "A total of " << lines_.size() << " images.";

  lines_id_ = 0;
  // Check if we would need to randomly skip a few data points
  if (this->layer_param_.image_data_param().rand_skip()) {
    unsigned int skip = blas_srng_rand() %
        this->layer_param_.image_data_param().rand_skip();
    LOG(INFO) << "Skipping first " << skip << " data points.";
    CHECK_GT(lines_.size(), skip) << "Not enough points to skip";
    lines_id_ = skip;
  }
  // Read an image, and use it to initialize the top blob.
  cv::Mat cv_img = ReadImageToCVMat(root_folder + lines_[lines_id_].first,
                                    new_height, new_width, is_color);
  CHECK(cv_img.data) << "Could not load " << lines_[lines_id_].first;
  // Use data_transformer to infer the expected blob shape from a cv_image.
  vector<int> top_shape = this->data_transformer_->InferBlobShape(cv_img);
  this->transformed_data_.Reshape(top_shape);
  // Reshape prefetch_data and top[0] according to the batch_size.
  const int batch_size = this->layer_param_.image_data_param().batch_size();
  CHECK_GT(batch_size, 0) << "Positive batch size required";
  top_shape[0] = batch_size;
  for (i = 0; i < this->prefetch_.size(); ++i) {
    this->prefetch_[i]->data_.Reshape(top_shape);
  }
  top[0]->Reshape(top_shape);

  LOG(INFO) << "output data size: " << top[0]->size.n << ","
      << top[0]->size.c << "," << top[0]->height() << ","
      << top[0]->width();
  // label
  vector<int> label_shape(1, batch_size);
  top[1]->Reshape(label_shape);
  for (i = 0; i < this->prefetch_.size(); ++i) {
    this->prefetch_[i]->label_.Reshape(label_shape);
  }
  if (layer->o_size == 3) {
    top[2]->Reshape(label_shape);
    for (i = 0; i < this->prefetch_.size(); ++i) {
      this->prefetch_[i]->weight_.Reshape(label_shape);
    }
    this->output_weights_ = true;
  }
}


void ImageDataLayer::ShuffleImages() {
  caffe::rng_t* prefetch_rng =
      static_cast<caffe::rng_t*>(prefetch_rng_->generator());
  shuffle(lines_.begin(), lines_.end(), prefetch_rng);
}

// This function is called on prefetch thread

void ImageDataLayer::load_batch(Batch* batch) {
  CPUTimer batch_timer;
  batch_timer.Start();
  double read_time = 0;
  double trans_time = 0;
  CPUTimer timer;
  CHECK(batch->data_.count());
  CHECK(this->transformed_data_.count());
  ImageDataParameter image_data_param = this->layer_param_.image_data_param();
  const int batch_size = image_data_param.batch_size();
  const int new_height = image_data_param.new_height();
  const int new_width = image_data_param.new_width();
  const bool is_color = image_data_param.is_color();
  string root_folder = image_data_param.root_folder();

  // Reshape according to the first image of each batch
  // on single input batches allows for inputs of varying dimension.
  cv::Mat cv_img = ReadImageToCVMat(root_folder + lines_[lines_id_].first,
      new_height, new_width, is_color);
  CHECK(cv_img.data) << "Could not load " << lines_[lines_id_].first;
  // Use data_transformer to infer the expected blob shape from a cv_img.
  vector<int> top_shape = this->data_transformer_->InferBlobShape(cv_img);
  this->transformed_data_.Reshape(top_shape);
  // Reshape batch according to the batch_size.
  top_shape[0] = batch_size;
  batch->data_.Reshape(top_shape);

  Dtype* prefetch_data = batch->data_;
  Dtype* prefetch_label = batch->label_;
  Dtype* prefetch_weight;
  if (this->output_weights_) {
    prefetch_weight = batch->weight_;
  }

  // datum scales
  const int lines_size = lines_.size();
  for (item_id = 0; item_id < batch_size; ++item_id) {
    // get a blob
    timer.Start();
    bool valid_sample = false;
    while (!valid_sample) {
      std::pair<std::string, int> this_line;

      if (balance_) {
        int pick_index = (blas_srng_rand() % num_samples_[class_id_]) + 1;
        for (auto& sample : filename_by_class_[class_id_]) {
          if (sample.second == 0) {
            pick_index--;
            if (pick_index == 0) {
              this_line = std::make_pair(sample.first, class_id_);
              sample.second = 1;
              num_samples_[class_id_]--;
              break;
            }
          }
        }
        CHECK_GT(this_line.first.size(), 0);
        if (num_samples_[class_id_] == 0) {
          num_samples_[class_id_] = filename_by_class_[class_id_].size();
          for (auto& sample : filename_by_class_[class_id_]) {
            sample.second = 0;
          }
        }
      }
      else {
        CHECK_GT(lines_size, lines_id_);
        this_line = lines_[lines_id_];
      }

      cv::Mat cv_img = ReadImageToCVMat(root_folder + this_line.first,
                                        new_height, new_width, is_color);
      if (!cv_img.data) {
        LOG(INFO) << "Could not load " << this_line.first;
        valid_sample = false;
      }
      else {
        valid_sample = true;
      }
      read_time += timer.MicroSeconds();
      timer.Start();
      // Apply transformations (mirror, crop...) to the image
      int offset = batch->data_.offset(item_id);
      this->transformed_data_.set_data(prefetch_data + offset);
      this->data_transformer_->Transform(cv_img, &(this->transformed_data_));
      trans_time += timer.MicroSeconds();

      prefetch_label[item_id] = this_line.second;
      if (this->output_weights_) {
        prefetch_weight[item_id] = class_weights_[this_line.second];
      }
      // go to the next iter
      if (balance_) {
        class_id_++;
        if (class_id_ >= num_samples_.size()) {
          // We have reached the end. Restart from the first.
          DLOG(INFO) << "Restarting data prefetching from start.";
          class_id_ = 0;
        }
      }
      else {
        lines_id_++;
        if (lines_id_ >= lines_size) {
          // We have reached the end. Restart from the first.
          DLOG(INFO) << "Restarting data prefetching from start.";
          lines_id_ = 0;
          if (this->layer_param_.image_data_param().shuffle()) {
            ShuffleImages();
          }
        }
      }
    }
  }
  batch_timer.Stop();
  DLOG(INFO) << "Prefetch batch: " << batch_timer.MilliSeconds() << " ms.";
  DLOG(INFO) << "     Read time: " << read_time / 1000 << " ms.";
  DLOG(INFO) << "Transform time: " << trans_time / 1000 << " ms.";
}

INSTANTIATE_CLASS(ImageDataLayer);
REGISTER_LAYER_CLASS(ImageData);


#endif  // USE_OPENCV