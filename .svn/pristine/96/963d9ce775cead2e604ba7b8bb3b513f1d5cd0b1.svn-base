var gulp = require('gulp');
var browserify = require('browserify');
var source = require("vinyl-source-stream");
var reactify = require('reactify');

gulp.task('browserify', function(){
  var b = browserify({
    entries: ['./jsx/index.jsx'],
    transform: [reactify]
  });
  return b.bundle()
    .pipe(source('index.js'))
    .pipe(gulp.dest('./public/js'));
});
