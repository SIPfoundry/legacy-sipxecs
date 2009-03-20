/*
 * XCF-3445, remove bean zipUpload, and change it to upload
 *
 */

update upload set bean_id='upload' where bean_id='zipUpload';
