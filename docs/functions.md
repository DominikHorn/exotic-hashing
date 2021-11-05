|      name         | perfect |        minimal      |     monotone     | monotone wrt. non-keys | order preserving | can recover key  | updateable | can directly store payloads |
|:-----------------:|:-------:|:-------------------:|:----------------:|:----------------------:|:----------------:|:----------------:|:----------:|:---------------------------:|
| bit mwhc          | &#9989; | almost (18.7% gaps) |                  |                        |                  |                  |            |                             |
| do nothing hash   | &#9989; |                     |                  |                        |                  |                  |            |                             |
| recsplit          | &#9989; | &#9989;             |                  |                        |                  |                  |            |                             |
| fst               | &#9989; | &#9989;             | &#9989;          | ?                      |                  | ?                |            |                             |
| compact trie      | &#9989; | &#9989;             | &#9989;          | &#9989;                |                  | &#9989;          | &#9989;    |                             |
| hollow trie       | &#9989; | &#9989;             | &#9989;          |                        |                  |                  |            |                             |
| learned linear    | &#9989; | &#9989;             | &#9989;          | possible               |                  |                  |            |                             |
| rank hash         | &#9989; | &#9989;             | &#9989;          | &#9989;                |                  | half recoverable |            |                             |
| learned rank hash | &#9989; | &#9989;             | &#9989;          | &#9989;                |                  | half recoverable |            |                             |
| la vector         | &#9989; | &#9989;             | &#9989;          | ?                      |                  | &#9989;          |            |                             |
| adaptive learned  | &#9989; | &#9989;             | &#9989;          | undetermined as of yet |                  | probably not     |            |                             |
| map omphf         | &#9989; | &#9989;             | depends on order |                        | &#9989;          |                  |            | &#9989;                     |
| mwhc              | &#9989; | &#9989;             | depends on order |                        | &#9989;          |                  |            |                             |
| sf mwhc           | &#9989; | &#9989;             | depends on order |                        | &#9989;          |                  |            | &#9989;                     |
